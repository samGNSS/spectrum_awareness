#include "cfar.h"

#include <cstring>
#include <volk/volk.h>
#include <iostream>
#include <ctime>
#include <chrono>

cfar::cfar(float pfa, int numCells, int numGuardBins, int buffLen):numCells(numCells),
                                                                     numGuardBins_(numGuardBins),buffLen(buffLen)
{
  forwardSlice = (float*)volk_malloc((this->numCells)*sizeof(float),volk_get_alignment());
  backwardsSlice = (float*)volk_malloc((this->numCells)*sizeof(float),volk_get_alignment());
  simdMath = new math(buffLen);
  log = new console();
  
  //calculate alpha
  this->alpha = numCells*(std::pow(pfa,-1.0/numCells) - 1.0);
  log->info(__FILENAME__,__LINE__,"CFAR alpha: %f",alpha);
  
}

cfar::~cfar(){
  volk_free(forwardSlice);
  volk_free(backwardsSlice);

  delete simdMath;
  delete log;
};

std::vector<radar::cfarDet> cfar::getDetections(radar::floatIQ* fftIn){
    //CFAR processing
    int bin             = 0;
    int binCheck        = buffLen>>1;
    int detBin          = 0;
    int cell            = 0;
    int forwardIdx      = 0;
    int backwardsIdx    = 0;
    float forwardMean   = 0;
    float backwardsMean = 0;
    float noiseEsti     = 0;
    radar::cfarDet tmpDet;

    std::vector<radar::cfarDet> dets;
    dets.reserve(buffLen);

    for(bin = 0;bin<buffLen;++bin){
        //get forwardSlice and backwardsSlice
        for(cell=0;cell<numCells;++cell){
            forwardIdx = (bin + numGuardBins_ + cell);
            forwardIdx > (buffLen-1) ? forwardIdx-=buffLen : forwardIdx;
            backwardsIdx = (bin - numGuardBins_ - cell);
            backwardsIdx < 0 ? backwardsIdx+=buffLen : backwardsIdx;
            forwardSlice[cell] = fftIn->iq[forwardIdx];
            backwardsSlice[cell] = fftIn->iq[backwardsIdx];
        }
        //get mean of forward slice
        simdMath->getMeanAndStdDev(forwardSlice,&forwardMean,numCells);
        //get mean of backwards slice
        simdMath->getMeanAndStdDev(backwardsSlice,&backwardsMean,numCells);

        //compare to signal level
        noiseEsti = (forwardMean+backwardsMean)*alpha;
        if(fftIn->iq[bin] > noiseEsti){
            //FFT shift
            bin>binCheck ? detBin = bin-buffLen : detBin = bin;

            tmpDet.fftBin          = detBin;
            tmpDet.power           = fftIn->iq[bin];
            tmpDet.freqHz          = fftIn->metaData.freqHz;
            tmpDet.timeOn          = std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now());
            tmpDet.timeOnMicroSec  = std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now())*1000000;

            //store detection
            dets.push_back(tmpDet);
        }
    }

    //log->info(__FILENAME__,__LINE__,"NumDets: %d",dets.size());
    return dets;
};
