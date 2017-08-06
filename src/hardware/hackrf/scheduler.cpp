/*
 * 
 * 
 * Handles the rx and tx scheduling, also passes rx buffers to the processing thread
 * 
 * 
 */

#include "scheduler.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace hackrf;

//forward declaration of static members
radar::charBuff* sched::rx_buff;
uint32_t sched::numRxSamps;
bool sched::ready;
bool sched::retuned;
bool sched::enabled;
proc* sched::pro;
console* sched::log;

sched::sched(){
    pro = new proc();
    log = new console();
}

sched::~sched(){
    //check threads have joined 
    if (enabled)
        this->stop(); //join threads

    //stop processor
    delete pro;

    log->info(__FILENAME__,__LINE__,"Stopped scheduler");

    delete log;
}


void sched::findDevices(){
  //enable and check the hardware
  int result = hackrf_init();
  if (result != HACKRF_SUCCESS){
    log->error(__FILENAME__,__LINE__,"No device found...stopping...");
    std::exit(-1);
  }else{
    log->info(__FILENAME__,__LINE__,"Found hackrf");
  }
  
  log->info(__FILENAME__,__LINE__,"Initializing hardware");

  result = hackrf_open(&hackrf);
  if (result != HACKRF_SUCCESS){
    log->error(__FILENAME__,__LINE__,"Failed to open the device...stopping...");
    std::exit(-1);
  }else{
    log->info(__FILENAME__,__LINE__,"Opened hackrf");
  }
}


//init hardware
void sched::init(sdr::deviceParams &frontEnd,sdr::scannerParams &scanner,sdr::detectorParams &detector){ 
  //set up device front end
  int result = set_up_device(&frontEnd,hackrf,log);
  if (result == -1){log->error(__FILENAME__,__LINE__,"Device set up failed");std::exit(-1);}
  log->info(__FILENAME__,__LINE__,"Return from device setup: %d",result);
  
  log->info(__FILENAME__,__LINE__,"Started Receiver");
  sleep(1);
  log->info(__FILENAME__,__LINE__,"Configuring sweep");
  sleep(1);
  //check number of tune bands, max number is 10
  if(scanner.numBands > 10){
    log->warn(__FILENAME__,__LINE__, "Too many scan bands, only using the first 10");
    scanner.numBands = 10;
  }else{
    log->info(__FILENAME__,__LINE__,"Using %d bands",scanner.numBands);
  };
  
  sleep(1);
  
  //get number of samples
  uint32_t num_samples = 0;
  int numSamps = frontEnd.sampRate * scanner.dwellTime;
  if(numSamps < 8192){
    log->warn(__FILENAME__,__LINE__,"Minimum number of samples is 8192");
    num_samples = 8192;
  }else if((numSamps) % 8192){
    log->warn(__FILENAME__,__LINE__,"Number of samples must be a multiple of 8192");
    num_samples = 8192*(int)std::ceil((float)numSamps / 8192.f);
    log->warn(__FILENAME__,__LINE__,"Number of samples: %d",num_samples);
  }
  
  sleep(1);
  
  int ret = hackrf_start_rx(hackrf, &sched::rx_callback, (void *) this);
  if (ret != HACKRF_SUCCESS){
    log->error(__FILENAME__,__LINE__,"Failed to start rx with error code: %d....stopping",ret);
    hackrf_exit();
    std::exit(-1);
  };
  
  //get frequency ranges
  int num_ranges = scanner.numBands;
  uint16_t frequencies[2*scanner.numBands];
  
  for(size_t i=0;i<scanner.numBands;++i){
    frequencies[2*i]   = (scanner.startFreq + scanner.bandwidth*i)/1e6;
    frequencies[2*i+1] = (scanner.startFreq + scanner.bandwidth*(i+1))/1e6;
    int step_count = 1 + (frequencies[2*i+1] - frequencies[2*i] - 1)/ 20;
    frequencies[2*i+1] = frequencies[2*i] + step_count * 20;
    log->debug(__FILENAME__,__LINE__,"Freq Range %d - %d",frequencies[2*i],frequencies[2*i+1]);
  }
  
  result = hackrf_init_sweep(this->hackrf, frequencies, num_ranges, num_samples*2, scanner.bandwidth, 0, LINEAR);
  
  log->info(__FILENAME__,__LINE__,"Return from sweep init: %d", result);
  
  //set ready to false
  ready = false;
  
  //set up processor
  pro->init(detector,frequencies[0]);
  
}

//start hardware
void sched::start(){
  //init flow controls
  log->info(__FILENAME__,__LINE__,"::STARTED SPECTRUM MONITOR::\n");
  sleep(5);
  enabled = true;
}

//stop hardware
void sched::stop(){
  enabled = false;
  //stop hackrf device
  hackrf_close(hackrf);
  hackrf_exit();
}

//---------------------Receiver-------------------------
int sched::rx_callback(hackrf_transfer* transfer){
  if(enabled){
    //receive
    rx_buff = reinterpret_cast<int8_t*>(transfer->buffer);
    
    pro->rx_monitor(rx_buff);
  }
  return 0;
};

