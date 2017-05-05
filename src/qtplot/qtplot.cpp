#include "qtplot.h"

#include <unistd.h>


qui::qui(){
  log = new console();
}

qui::~qui(){
  if(enabled)
    this->stop();
  
  delete log;
}

void qui::stop(){
  enabled = false;
  queueMonitor.join();
}


void qui::init(){
  enabled = true;
  queueMonitor = std::thread(std::bind(&qui::watchQueue, this));
}


void qui::watchQueue(){
  while(enabled){
     std::unique_lock<std::mutex> lk(disPlayMtx);
     
     //watch queue for new detections
     while(detQueue.empty()){usleep(100);};
     
     while(!detQueue.empty()){
       dets = detQueue.front();
       for(auto det : dets){
         log->info(__FILENAME__,__LINE__,"Det freq: %llu \t Det bin: %d", det.freqHz,det.startBin);
       }
       detQueue.pop();  
    }
     disPlayMtx.unlock();
  }
}


std::queue<std::vector<radar::cfarDet>>* qui::getQueue(){
  return &detQueue;
};

