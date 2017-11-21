#include "processor.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <memory>
#include <unistd.h>

#include <ctime>

using namespace hackrf;

processor::processor(){
    //memory buffer
    // memBuffer = memBuff::getInst();
    m_numDets = 0;

#ifdef debug
    this->dbgFile = std::ofstream("fft.bin",std::ios::binary | std::ios::app);
#endif
}

processor::~processor(){
    if(m_enabled)
        Stop();

    delete m_fftProc;
    delete m_cfarFilt;
    //delete memBuffer;

    console::info(__FILENAME__,__LINE__,"Stopped processor");

    m_fftBuffs.clear();
    m_absBuffs.clear();
    m_floatBuffs.clear();
}

void processor::Stop(){
    m_enabled = false;
    //usleep(100);
    m_buffRdy = true;
    m_waitForBuff.notify_one();

    //join threads
    m_detThread.join();
    m_monitorThread.join();
}


void processor::Init(sdr::detectorParams &detector, uint16_t startFreq, uint16_t endFreq)
{
    m_startFreq = startFreq;
    m_endFreq   = endFreq;

    //Thread controls
    m_enabled      = true;
    m_buffRdy      = false;
    m_waiting      = false;
    m_sweepStarted = false;

    //fft object
    m_fftProc = new FFT(detector.fftSize, detector.fftSize, detector.windowType);

    //cfar detector
    m_cfarFilt = new cfar(detector.pfa, detector.numAvgBins, detector.numGuardBins, detector.fftSize, detector.freqSlip, 20000000);

    //allocate buffs
    m_floatBuffs.resize(m_blocksPerTransfer);
    m_fftBuffs.resize(m_blocksPerTransfer);
    m_absBuffs.resize(m_blocksPerTransfer);
    for(int buff=0;buff<m_blocksPerTransfer;++buff){
        //pre-fft
        m_floatBuffs[buff] = new radar::cfloatIQ(BYTES_PER_BLOCK>>1);
        m_floatBuffs[buff]->buffSize = BYTES_PER_BLOCK>>1;
        m_floatBuffs[buff]->metaData.valid = false;

        //post-fft
        m_absBuffs[buff] = new radar::floatIQ(detector.fftSize);
        m_absBuffs[buff]->buffSize = BYTES_PER_BLOCK>>1;
        m_absBuffs[buff]->metaData.valid = false;
    }

    //bind threads
    m_detThread = std::thread(std::bind(&processor::SignalInt, this));
    m_monitorThread = std::thread(std::bind(&processor::SystemMonitor, this));

}

void processor::RxMonitor(radar::charBuff* rx_buff)
{
    uint64_t frequency;
    for(int band=0;band<m_blocksPerTransfer;++band){
        uint8_t* uBuff = (uint8_t*)rx_buff;

        //get the freqency of the buffer
        if(uBuff[0] == 0x7F && uBuff[1] == 0x7F){
            frequency = ((uint64_t)(uBuff[9]) << 56) | ((uint64_t)(uBuff[8]) << 48) | ((uint64_t)(uBuff[7]) << 40)
                | ((uint64_t)(uBuff[6]) << 32) | ((uint64_t)(uBuff[5]) << 24) | ((uint64_t)(uBuff[4]) << 16)
                | ((uint64_t)(uBuff[3]) << 8) | uBuff[2];
        } else {
            rx_buff += BYTES_PER_BLOCK;
            continue;
        }

        //check if we started the sweep
        if(frequency==(uint64_t)(m_startFreq*1e6)){
            m_sweepStarted = true;
        }

        //check if we are currently in the middle of a stream; if we are, wait for it to start
        if(!m_sweepStarted){
            rx_buff += BYTES_PER_BLOCK;
            continue;
        }

        //invalid frequency
        if(frequency > m_endFreq) {
			rx_buff += BYTES_PER_BLOCK;
			continue;
		}

        math::interleavedUCharToComplexFloat(rx_buff, m_floatBuffs[band]->iq,BYTES_PER_BLOCK>>1);
        m_floatBuffs[band]->metaData.valid  = true;
        m_floatBuffs[band]->metaData.freqHz = frequency;
        m_floatBuffs[band]->metaData.time   = std::time(0);

        // console::warn(__FILENAME__,__LINE__,"buff frequency: %llu",frequency);


        rx_buff += BYTES_PER_BLOCK;
    }

    //send buffer to the detector
    if(m_waiting){
        m_buffRdy = true;
        m_waitForBuff.notify_one();
    }else{
        console::warn(__FILENAME__, __LINE__, "Proc overflow!");
    }

}

void processor::SignalInt()
{
    int band;
    while(m_enabled){
        //lock mutex
        std::unique_lock<std::mutex> lk(m_buffMutex);
        //wait for cv notify
        m_waiting = true;
        m_waitForBuff.wait(lk, [&]{return m_buffRdy;});
        m_waiting = false;
        m_buffRdy = false;
        if(m_enabled){
            //get fft
            for(band=0;band<m_blocksPerTransfer;++band){
                if(m_floatBuffs[band]->metaData.valid){
                    //set meta data
                    m_absBuffs[band]->metaData.freqHz = m_floatBuffs[band]->metaData.freqHz;
                    m_absBuffs[band]->metaData.time   = m_floatBuffs[band]->metaData.time;
                    m_absBuffs[band]->metaData.valid  = true;

                    //get fft
                    m_fftProc->getFFT_ABS(m_floatBuffs[band]->iq, m_absBuffs[band]->iq);

#ifdef debug
int32_t tmp = 1024;
dbgFile.write(reinterpret_cast<const char*>(&tmp), sizeof(int32_t));
dbgFile.write(reinterpret_cast<const char*>(m_absBuffs[band]->iq), 1024*sizeof(float));
#endif

                    //send to detector
                    m_procDets = m_cfarFilt->getDetections(m_absBuffs[band]);

                    //store in buffer
                    m_numDets += m_procDets.size();

                    m_floatBuffs[band]->metaData.valid = false;
                    m_procDets.clear();
                }
            }
        }
        lk.unlock();
    }
}

void processor::SystemMonitor(){
    while(m_enabled){
        console::info(__FILENAME__,__LINE__,"Number of detections: %d",m_numDets);
        m_numDets = 0;
        sleep(1);
    }
}
