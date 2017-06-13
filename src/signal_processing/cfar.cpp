#include "cfar.h"

#include <cstring>
#include <volk/volk.h>
#include <iostream>
#include <time.h>

cfar::cfar(float alpha, int numCells, int numGuardBins, int buffLen, int numAvg):alpha(alpha),
                                                     numCells(numCells/2),
                                                     numGuardBins_(numGuardBins),
                                                     buffLen(buffLen),numAvg(numAvg)
{
  forwardSlice = (float*)volk_malloc((this->numCells)*sizeof(float),volk_get_alignment());
  backwardsSlice = (float*)volk_malloc((this->numCells)*sizeof(float),volk_get_alignment());
  simdMath = new math(buffLen);
  log = new console();
}
  
cfar::~cfar(){
  volk_free(forwardSlice);
  volk_free(backwardsSlice);
  
  delete simdMath;
  delete log;
};

std::vector<radar::cfarDet> cfar::getDetections(radar::floatIQ* fftIn){
    //CFAR processing
    int bin = 0;
    //int binCheck = buffLen>>1;
    int cell = 0;
    float forwardMean = 0;
    float backwardsMean = 0;
    float noiseEsti = 0;
    radar::cfarDet tmpDet;
    std::vector<radar::cfarDet> dets;
    dets.reserve(buffLen);

    for(;bin<buffLen;++bin){
        //get forwardSlice and backwardsSlice
        for(cell=0;cell<numCells;++cell){
            forwardSlice[cell] = fftIn->iq[(bin + numGuardBins_ + cell)%buffLen];
            backwardsSlice[cell] = fftIn->iq[(bin - numGuardBins_ - cell)%buffLen];
        }
        //get mean of forward slice
        simdMath->getMeanAndStdDev(forwardSlice,&forwardMean,numCells);
        //get mean of backwards slice
        simdMath->getMeanAndStdDev(backwardsSlice,&backwardsMean,numCells);
        //compare to signal level
        noiseEsti = (forwardMean+backwardsMean)*alpha;
        if(fftIn->iq[bin] > noiseEsti){
            tmpDet.startBin = bin;
            tmpDet.stopBin  = bin;
            tmpDet.power    = 20*log10(fftIn->iq[bin]);
            tmpDet.freqHz   = fftIn->metaData.freqHz;
            tmpDet.time     = std::time(0);
            dets.push_back(tmpDet);
        }
    }
    //log->info(__FILENAME__,__LINE__,"NumDets: %d",dets.size());
    return dets;
};
