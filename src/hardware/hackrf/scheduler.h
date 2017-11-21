#ifndef __HACKRF_SCHED_H__
#define __HACKRF_SCHED_H__

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "processor.h"
#include "driver/hackrf.h"
#include "driver/device_setup.h"
#include "../HardwareBase/baseSched.h"
#include "../../util/radarDataTypes.h"
#include "../../util/logger/consoleLog.h"

//TODO: implement some kind of common time base, could use the system time but it seems bad...
namespace hackrf{
  class scheduler : baseScheduler{
    public:
      scheduler();
      ~scheduler();
      void init(sdr::deviceParams &frontEnd,sdr::scannerParams &scanner,sdr::detectorParams &detector);          //init hardware
      void start();         //start threads
      void stop();          //stop threads
      void findDevices();   //find all sdrs attached to the computer

    private:
      //private methods
      void rx_callback_control();   //handle rx
      static int rx_callback(hackrf_transfer* transfer);

      //hackrf variables
      hackrf_device* hackrf; 		//device pointer

      //buffers and things...
      static radar::charBuff* rx_buff;
      static uint32_t numRxSamps;
      uint32_t numTotalSamps;
      std::vector<radar::charIQ*> iqBuffs;
      int_fast32_t band;
      int_fast32_t buff;
      int_fast64_t numSamps;
      static processor* pro;

      //thread controls
      static bool enabled;
      static bool ready;
      static bool retuned;

      static constexpr int sampleMultiple = 8192; 

  };
}



#endif
