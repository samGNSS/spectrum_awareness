#include "processor.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <memory>
#include <unistd.h>

#include <ctime>

using namespace hackrf;

proc::proc(){
    //memory buffer
    memBuffer   = memBuff::getInst();

    //colorized print outs
    log = new console();

    numDets = 0;
}

proc::~proc(){
    if(enabled)
        stop();

    delete fftProc;
    delete simdMath;
    delete cfarFilt;
    //delete memBuffer;

    log->info(__FILENAME__,__LINE__,"Stopped processor");
    delete log;

    fftBuffs.clear();
    absBuffs.clear();
    floatBuffs.clear();
}

void proc::stop(){
    enabled = false;
    usleep(100);
    buffRdy = true;
    waitForBuff.notify_one();

    //join threads
    detThread.join();
    monitorThread.join();
}


void proc::init(sdr::detectorParams &detector, uint16_t startFreq)
{
    this->startFreq = startFreq;

    //Thread controls
    enabled = true;
    buffRdy = false;
    waiting = false;
    sweepStarted = false;

    //fft object
    fftProc = new FFT(detector.fftSize,detector.fftSize,detector.windowType);

    //math object
    simdMath = new math(detector.fftSize);

    //cfar detector
    cfarFilt = new cfar(detector.pfa,detector.numAvgBins,detector.numGuardBins,detector.fftSize);

    //allocate buffs
    floatBuffs.resize(blocksPerTransfer);
    fftBuffs.resize(blocksPerTransfer);
    absBuffs.resize(blocksPerTransfer);
    for(int buff=0;buff<blocksPerTransfer;++buff){
        floatBuffs[buff] = new radar::cfloatIQ(BYTES_PER_BLOCK>>1);
        floatBuffs[buff]->buffSize = BYTES_PER_BLOCK>>1;
        floatBuffs[buff]->metaData.valid = false;
        fftBuffs[buff] = new radar::cfloatIQ(detector.fftSize);
        absBuffs[buff] = new radar::floatIQ(detector.fftSize);
        absBuffs[buff]->buffSize = BYTES_PER_BLOCK>>1;
        absBuffs[buff]->metaData.valid = false;
    }

    //bind threads
    detThread = std::thread(std::bind(&proc::signal_int, this));
    monitorThread = std::thread(std::bind(&proc::systemMonitor, this));

}

void proc::rx_monitor(radar::charBuff* rx_buff)
{
    uint64_t frequency;
    for(int band=0;band<blocksPerTransfer;++band){
        uint8_t* uBuff = (uint8_t*)rx_buff;
        if(uBuff[0] == 0x7F && uBuff[1] == 0x7F){
            frequency = ((uint64_t)(uBuff[9]) << 56) | ((uint64_t)(uBuff[8]) << 48) | ((uint64_t)(uBuff[7]) << 40)
        | ((uint64_t)(uBuff[6]) << 32) | ((uint64_t)(uBuff[5]) << 24) | ((uint64_t)(uBuff[4]) << 16)
        | ((uint64_t)(uBuff[3]) << 8) | uBuff[2];

        } else {
            rx_buff += BYTES_PER_BLOCK;
            continue;
        }

        if(frequency==(uint64_t)(startFreq*1e6) && !sweepStarted){
            sweepStarted = true;
        }

        if(!this->sweepStarted){
            rx_buff += BYTES_PER_BLOCK;
            continue;
        }

        simdMath->interleavedUCharToComplexFloat(rx_buff,floatBuffs[band]->iq,BYTES_PER_BLOCK>>1);
        floatBuffs[band]->metaData.valid  = true;
        floatBuffs[band]->metaData.freqHz = frequency;
        floatBuffs[band]->metaData.time   = std::time(0);

        rx_buff += BYTES_PER_BLOCK;
    }

    //send buffer to the detector
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
            for(band=0;band<blocksPerTransfer;++band){
                if(floatBuffs[band]->metaData.valid){
                    //set meta data
                    absBuffs[band]->metaData.freqHz = floatBuffs[band]->metaData.freqHz;
                    absBuffs[band]->metaData.time   = floatBuffs[band]->metaData.time;
                    absBuffs[band]->metaData.valid  = true;

                    //get fft
                    fftProc->getFFT(floatBuffs[band]->iq,fftBuffs[band]->iq);
                    simdMath->magSqrd(fftBuffs[band]->iq,absBuffs[band]->iq);

                    //send to detector
                    procDets = cfarFilt->getDetections(absBuffs[band]);
                    numDets += procDets.size();

                    //store in buffer
                    if((procDets.size() > 0) & (procDets.size() < 600)){
                        //udp->pubDets(procDets);
                        memBuffer->publishDets(procDets);
                    }

                    floatBuffs[band]->metaData.valid = false;
                }
            }
        }
        lk.unlock();
    }
}

void proc::systemMonitor(){
    while(enabled){
        log->info(__FILENAME__,__LINE__,"Number of detections: %d",numDets);
        numDets = 0;
        sleep(10);
    }
}
