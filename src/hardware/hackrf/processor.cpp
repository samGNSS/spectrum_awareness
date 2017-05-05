#include "processor.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <memory>
#include <unistd.h>

using namespace hackrf;

proc::proc(){}

proc::~proc(){
  if(enabled)
    stop();
  
  delete fftProc;
  delete simdMath;
  delete cfarFilt;
  delete log;
}

void proc::stop(){
  this->enabled = false;
  usleep(100);
  this->buffRdy = true;
  waitForBuff.notify_one();
  //join threads
  this->detThread.join();
}


void proc::init(int fftSize, int inputSize, int numBands, uint16_t startFreq)
{
  this->enabled = true;
  this->buffLen = inputSize;
  this->startFreq = startFreq;
  this->sweepStarted = false;
  
  fftProc     = new FFT(fftSize,inputSize);
  simdMath    = new math(fftSize);
  quiH        = new qui();
  cfarFilt    = new cfar(quiH->getQueue(),1.5*4,200,5,fftSize);
  log         = new console();
  
  buffRdy = false;
  waiting = false;
  buffNum = 0;
  procNum = 0;
  
  //bind threads
  this->detThread = std::thread(std::bind(&proc::signal_int, this));
  
  this->numBands = numBands;
  
  //allocate buffs
  floatBuffs.resize(numBands);
  fftBuffs.resize(numBands);
  absBuffs.resize(numBands);
  for(int buff=0;buff<numBands;++buff){
    floatBuffs[buff] = new radar::cfloatIQ(inputSize);
    floatBuffs[buff]->metaData.valid = false;
    fftBuffs[buff] = new radar::cfloatIQ(fftSize); 
    absBuffs[buff] = new radar::floatIQ(fftSize); 
  }
  
  //init gui
  quiH->init();
  
}

void proc::rx_monitor(radar::charBuff* rx_buff)
{
  //check if the buffer is valid then...
  //got a buffer, convert to complex float and get fft
  uint64_t frequency;
  for(int band=0;band<numBands;++band){
    if(rx_buff[0] == 0x7F && rx_buff[1] == 0x7F){
      frequency = ((uint64_t)(rx_buff[9]) << 56) | ((uint64_t)(rx_buff[8]) << 48) | ((uint64_t)(rx_buff[7]) << 40)
	| ((uint64_t)(rx_buff[6]) << 32) | ((uint64_t)(rx_buff[5]) << 24) | ((uint64_t)(rx_buff[4]) << 16)
	| ((uint64_t)(rx_buff[3]) << 8) | rx_buff[2];
	
    } else {
//       log->warn(__FILENAME__,__LINE__,"Invalid samples??");
      rx_buff += buffLen;
      continue;
    }
    
    if(frequency==(uint64_t)(startFreq*1e6)){
      log->info(__FILENAME__,__LINE__,"Buffer frequency: %d",frequency);
      this->sweepStarted = true;
    }
    if(!this->sweepStarted){
      rx_buff += buffLen;
      continue;
    }
    
    rx_buff += buffLen;
    for(int i=0,j=0;i<buffLen;i+=2,++j){
      floatBuffs[band]->iq[j] = radar::complexFloat(rx_buff[i],rx_buff[i+1]);
      floatBuffs[band]->iq[j] /= 128;
      floatBuffs[band]->metaData.valid = true;
    }
    rx_buff += buffLen;
  }
  if(waiting){
    buffRdy = true;
    waitForBuff.notify_one();
  }else{
    log->warn(__FILENAME__,__LINE__,"Proc overflow!!");
  }
}

void proc::signal_int()
{
  int band;
  while(enabled){
    //lock mutex
    std::unique_lock<std::mutex> lk(buffMutex);
    //wait for cv notify
    waiting = true;
    waitForBuff.wait(lk, [&]{return buffRdy;});
    waiting = false;
    buffRdy = false;
    if(enabled){
      //get fft
      for(band=0;band<numBands;++band){
	if(floatBuffs[band]->metaData.valid){
	  fftProc->getFFT(floatBuffs[band]->iq,fftBuffs[band]->iq);
	  simdMath->abs(fftBuffs[band]->iq,absBuffs[band]->iq);
	  cfarFilt->getDetections(absBuffs[band]);
	}
      }
    }
    lk.unlock();
  }
}
