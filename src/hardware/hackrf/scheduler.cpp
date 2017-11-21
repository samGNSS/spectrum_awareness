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
radar::charBuff* scheduler::rx_buff;
uint32_t scheduler::numRxSamps;
bool scheduler::ready;
bool scheduler::retuned;
bool scheduler::enabled;
processor* scheduler::pro;

scheduler::scheduler(){
    pro = new processor();
}

scheduler::~scheduler(){
    //check threads have joined
    if (enabled)
        this->stop(); //join threads

    //stop processor
    delete pro;

    console::info(__FILENAME__,__LINE__,"Stopped scheduler");
}


void scheduler::findDevices(){
    //enable and check the hardware
    int result = hackrf_init();
    if (result != HACKRF_SUCCESS){
        console::error(__FILENAME__,__LINE__,"No device found...stopping...");
        std::exit(-1);
    }else{
        console::info(__FILENAME__,__LINE__,"Found hackrf");
    }

    console::info(__FILENAME__,__LINE__,"Initializing hardware");

    result = hackrf_open(&hackrf);
    if (result != HACKRF_SUCCESS){
        console::error(__FILENAME__,__LINE__,"Failed to open the device...stopping...");
        std::exit(-1);
    }else{
        console::info(__FILENAME__,__LINE__,"Opened hackrf");
    }
}


//init hardware
void scheduler::init(sdr::deviceParams &frontEnd,sdr::scannerParams &scanner,sdr::detectorParams &detector){
    //set up device front end
    int result = set_up_device(&frontEnd,hackrf);
    if (result == -1){
        console::error(__FILENAME__,__LINE__,"Device set up failed");std::exit(-1);
    }

    console::info(__FILENAME__,__LINE__,"Return from device setup: %d",result);
    console::info(__FILENAME__,__LINE__,"Starting Receiver");
    sleep(1);
    console::info(__FILENAME__,__LINE__,"Configuring sweep, starting freqency is: %llu",scanner.startFreq);
    //check number of tune bands, max number is 10
    if(scanner.numBands > 10){
        console::warn(__FILENAME__,__LINE__, "Too many scan bands, only using the first 10");
        scanner.numBands = 10;
    }else{
        console::info(__FILENAME__,__LINE__,"Using %d bands",scanner.numBands);
    };

    sleep(1);

    //get number of samples
    uint32_t num_samples = 0;
    int numSamps = frontEnd.sampRate * scanner.dwellTime;
    if(numSamps < sampleMultiple){
        console::warn(__FILENAME__,__LINE__,"Minimum number of samples is 8192");
        num_samples = sampleMultiple;
    }else if((numSamps) % sampleMultiple){
        num_samples = sampleMultiple*(int)std::ceil((float)numSamps / (float)sampleMultiple);
        console::warn(__FILENAME__,__LINE__,"Number of samples must be a multiple of %d \n\t using samples: %d",sampleMultiple,num_samples);
    }

    sleep(1);

    //start the receiver
    int ret = hackrf_start_rx(hackrf, &scheduler::rx_callback, (void *) this);
    if (ret != HACKRF_SUCCESS){
        console::error(__FILENAME__,__LINE__,"Failed to start rx with error code: %d....stopping",ret);
        hackrf_exit();
        std::exit(-1);
    };

    //if the scanner is enabled, build a list of tune bands from the starting frequency to the end spaced apart by the sample rate
    if(scanner.enabled){
        //get frequency ranges
        int num_ranges = scanner.numBands;
        uint16_t frequencies[2*scanner.numBands];

        for(size_t i=0;i<(size_t)scanner.numBands;++i){
            frequencies[2*i]   = (scanner.startFreq + scanner.bandwidth*i)/1e6;     //start
            frequencies[2*i+1] = (scanner.startFreq + scanner.bandwidth*(i+1))/1e6; //end

            //make sure start and stop are seperated by a multiple of the bandwidth
            int step_count     = 1 + (frequencies[2*i+1] - frequencies[2*i] - 1)/ 20;
            frequencies[2*i+1] = frequencies[2*i] + step_count * 20; ///TODO: don't hardcode the step size
        }

        //program the sweep
        result = hackrf_init_sweep(this->hackrf, frequencies, num_ranges, num_samples*2, scanner.bandwidth, 0, LINEAR);
        console::info(__FILENAME__,__LINE__,"Return from sweep init: %d", result);

        //set up processor
        console::info(__FILENAME__,__LINE__,"Starting processor");
        pro->Init(detector, frequencies[0], frequencies[2*scanner.numBands - 1]);
    }else{
        //set up processor
        console::info(__FILENAME__,__LINE__,"Starting processor");
        pro->Init(detector, frontEnd.centerFreq, frontEnd.centerFreq);
    }

    //set ready to false, wait for start
    ready = false;
} //init

//start hardware
void scheduler::start(){
    //init flow controls
    console::info(__FILENAME__,__LINE__,"::STARTED SPECTRUM MONITOR::\n");
    sleep(1);
    enabled = true;
}

//stop hardware
void scheduler::stop(){
    enabled = false;

    //stop hackrf device
    hackrf_close(hackrf);
    hackrf_exit();
}

//---------------------Receiver-------------------------
int scheduler::rx_callback(hackrf_transfer* transfer){
    if(enabled){
        //receive
        rx_buff = reinterpret_cast<int8_t*>(transfer->buffer);

        //give the buffer to the processor
        pro->RxMonitor(rx_buff);
    }
    return 0;
};
