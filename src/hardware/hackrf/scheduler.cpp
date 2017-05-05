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

sched::sched(const sdr::device_params* device_options){
  frontEnd = device_options;
  log = new console();
}

sched::~sched(){
  //check threads have joined 
  if (enabled)
    this->stop(); //join threads
    
  delete pro;
  
  log->info(__FILENAME__,__LINE__,":STOPPED SPECTRUM MONITOR::\n");

  
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
  
}


//init hardware
void sched::init(std::vector<radar::freqRange> scanBands, float dwellTime){ 
  log->info(__FILENAME__,__LINE__,"Initializing hardware");

  int result = hackrf_open(&hackrf);
  if (result != HACKRF_SUCCESS){
    log->error(__FILENAME__,__LINE__,"Failed to open the device...stopping...");
    std::exit(-1);
  }else{
    log->info(__FILENAME__,__LINE__,"Opened hackrf");
  }
  
  //set up device front end
  result = set_up_device(this->frontEnd,hackrf,log);
  if (result == -1){log->error(__FILENAME__,__LINE__,"Device set up failed");std::exit(-1);}
  log->info(__FILENAME__,__LINE__,"Return from device setup: %d",result);
//   std::cout << "Return from device setup: " << result << "\n";
  
  int ret = hackrf_start_rx(hackrf, &sched::rx_callback, (void *) this);
  if (ret != HACKRF_SUCCESS){
    log->error(__FILENAME__,__LINE__,"Failed to start rx with error code: %d....stopping",ret);
    hackrf_exit();
    std::exit(-1);
  };
  
  log->info(__FILENAME__,__LINE__,"Started Receiver");
  
  log->info(__FILENAME__,__LINE__,"Configuring sweep");
  //check number of tune bands, max number is 10
  if(scanBands.size() > 10){
    log->warn(__FILENAME__,__LINE__, "Too many scan bands, only using the first 10");
    scanBands.resize(10);
  }else{
    log->info(__FILENAME__,__LINE__,"Using %d bands",scanBands.size());
  };
  
  //set up call to init_sweep
  //get number of samples
  uint32_t num_samples = 0;
  if(this->frontEnd->sampRate * dwellTime < 8192){
    log->warn(__FILENAME__,__LINE__,"Minimum number of samples is 8192");
    num_samples = 8192;
  }else if((int)(this->frontEnd->sampRate * dwellTime) % 8192){
    log->warn(__FILENAME__,__LINE__,"Number of samples must be a multiple of 8192");
    num_samples = 8192*(int)std::ceil(this->frontEnd->sampRate * dwellTime / 8192);
  }
  
  int num_ranges = scanBands.size();
//   int interleaved = 0;
  uint16_t frequencies[2*scanBands.size()];
  for(uint i=0;i<scanBands.size();++i){
    frequencies[2*i]   = scanBands[i].first/1e6;
    frequencies[2*i+1] = scanBands[i].second/1e6;
    int step_count = 1 + (frequencies[2*i+1] - frequencies[2*i] - 1)
				/ 20;
     frequencies[i+1] = frequencies[2*i] + step_count * 20;
    log->debug(__FILENAME__,__LINE__,"Freq Range %d - %d",frequencies[i],frequencies[i+1]);
  }
  
  result = hackrf_init_sweep(this->hackrf, frequencies, num_ranges, num_samples * 2,
			20000000, 7500000, INTERLEAVED);
  
  log->info(__FILENAME__,__LINE__,"Return from sweep init: %d", result);
  
  //set ready to false
  ready = false;
  
  //set up processor
  pro = new proc();
  pro->init(1024,num_samples,num_ranges,frequencies[0]);
}

//start hardware
void sched::start(){
  //init flow controls
  enabled = true;
  log->info(__FILENAME__,__LINE__,"::STARTED SPECTRUM MONITOR::\n");
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
    rx_buff = (transfer->buffer);
    pro->rx_monitor(rx_buff);
  }else{
    log->warn(__FILENAME__,__LINE__, "Not enabled");
  }
  
  return 0;
};

