#include "cfar.h"

#include <cstring>
#include <volk/volk.h>
#include <iostream>
//#include <ctime>
#include <chrono>

cfar::cfar(float pfa, int numCells, int numGuardBins, int buffLen, int freqSlip, int sampleRate):m_numCells(numCells),
                                                                m_numGuardBins(numGuardBins),m_buffLen(buffLen),
                                                                m_freqSlip(freqSlip),
                                                                m_sampleRate(sampleRate)
{
    m_freqReso = (m_sampleRate/m_buffLen);
    m_freqSlipHz = m_freqSlip * m_freqReso;

    //calculate alpha
    m_alpha = numCells*(std::pow(pfa,-1.0/numCells) - 1.0);
    console::info(__FILENAME__,__LINE__,"CFAR alpha: %f",m_alpha);

    time = new timer();
    std::pair<uint,uint> timeVal = time->getTime();
    console::info(__FILENAME__,__LINE__,"CFAR Starting at time: %llu",timeVal.first);

    dbgFile.open("cfar_noise_estimate.bin",std::ios::binary);

}

cfar::~cfar(){
    dbgFile.close();
    delete time;
};

void cfar::InitTrainingCells(float* input){

    m_forwardNoiseEstimate = 0; m_forwardStart = m_numGuardBins; m_forwardEnd = m_numCells + m_numGuardBins - 1;
    m_reverseNoiseEstimate = 0; m_reverseStart = m_buffLen - m_numCells - m_numGuardBins; m_reverseEnd = m_buffLen - m_numGuardBins - 1;

    size_t idx1,idx2;
    for(idx1 = m_forwardStart,idx2 = m_reverseStart; idx1 <= m_forwardEnd; ++idx1,++idx2){
        m_forwardNoiseEstimate += input[idx1];
        m_reverseNoiseEstimate += input[idx2];
    }
}

std::vector<radar::target> cfar::getDetections(radar::floatIQ* fftIn){
    //CFAR processing
    int bin             = 0;
    int binCheck        = m_buffLen>>1;
    int detBin          = 0;
    float mean          = 0;
    float noiseEsti     = 0;
    radar::cfarDet tmpDet;

    std::vector<radar::cfarDet> dets;
    std::vector<radar::target>  targets;
    dets.reserve(m_buffLen);

    InitTrainingCells(fftIn->iq);

    //loop through buffer
    for(bin = 0;bin<m_buffLen;++bin){
        mean = std::abs(m_forwardNoiseEstimate + m_reverseNoiseEstimate);
        //compare to signal level
        noiseEsti = mean*m_alpha/(2*m_numCells);

        dbgFile.write((char*)&noiseEsti,sizeof(float));

        // console::debug(__FILENAME__,__LINE__, "Input mag: %lf, Noise estimate: %lf",fftIn->iq[bin],noiseEsti);


        if(fftIn->iq[bin] > noiseEsti){
            std::pair<uint,uint> timeVal = time->getTime();
            //FFT shift
            bin > binCheck ? detBin = bin - m_buffLen : detBin = bin;

            uint64_t freqOffset = fftIn->metaData.freqHz + (bin)*m_freqReso;

            tmpDet.power           = fftIn->iq[bin];
            tmpDet.freqHz          = freqOffset;
            tmpDet.timeOn          = timeVal.first;
            tmpDet.timeOnMicroSec  = timeVal.second;

            //store detection
            dets.push_back(tmpDet);
        }

        m_forwardNoiseEstimate -= fftIn->iq[m_forwardStart];
        m_reverseNoiseEstimate -= fftIn->iq[m_reverseStart];

        m_forwardStart = (m_forwardStart + 1) % m_buffLen;
        m_reverseStart = (m_reverseStart + 1) % m_buffLen;

        m_forwardNoiseEstimate += fftIn->iq[++m_forwardEnd % m_buffLen];
        m_reverseNoiseEstimate += fftIn->iq[++m_reverseEnd % m_buffLen];

    }

    //run compaction
    targets = compaction(dets);

    return targets;
};

std::vector<radar::target> cfar::compaction(std::vector<radar::cfarDet> dets){
    //assuming no frequency slip, the maximum number of dets we can have is the same size dets
    std::vector<radar::target> targets; targets.reserve(dets.size());

    for(auto& det : dets){
        //check if the frequecny ranges overlap
        if(targets.size() == 0){
            targets.push_back(newTarget(det));
        }

        radar::target& currTarget = targets.back();

        //check if the detection overlaps with the current target
        uint64_t low  = currTarget.freqHz.first - (currTarget.freqHz.second>>1);
        uint64_t high = currTarget.freqHz.first + (currTarget.freqHz.second>>1);

        if((high + m_freqSlipHz) > det.freqHz && (low - m_freqSlipHz) < det.freqHz){
            currTarget = addDetToTarget(currTarget, det);
        }else{
            targets.push_back(newTarget(det));
        }
    }

    return targets;
}

radar::target cfar::newTarget(radar::cfarDet det){
    radar::target tmp;
    tmp.freqHz.first   = det.freqHz;
    tmp.freqHz.second  = 0;
    tmp.power          = det.power;
    tmp.timeOn         = det.timeOn;
    tmp.timeOnMicroSec = det.timeOnMicroSec;
    return tmp;
}

radar::target cfar::addDetToTarget(radar::target target, radar::cfarDet det){
    uint64_t low  = std::min(target.freqHz.first - (target.freqHz.second>>1),det.freqHz);
    uint64_t high = std::max(target.freqHz.first + (target.freqHz.second>>1),det.freqHz);

    target.freqHz.first   = (high+low)>>1;
    target.freqHz.second  = (high-low);
    target.power          += det.power;
    target.timeOn         = std::max(det.timeOn,target.timeOn);
    target.timeOnMicroSec = std::min(det.timeOnMicroSec,target.timeOnMicroSec);

    return target;
}
