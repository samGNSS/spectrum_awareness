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
    int cell = 0;
    float forwardMean = 0;
    float backwardsMean = 0;
    float noiseEsti = 0;
    radar::cfarDet tmpDet;
    std::vector<radar::cfarDet> dets;
    dets.reserve(1024);

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
            //detection
            if(bin < buffLen/2){
                tmpDet.startBin = bin + buffLen/2;
                tmpDet.stopBin  = bin + buffLen/2;
            }else{
                tmpDet.startBin = bin - buffLen/2;
                tmpDet.stopBin  = bin - buffLen/2;
            }
            tmpDet.power    = 20*log10(fftIn->iq[bin]);
            tmpDet.freqHz   = fftIn->metaData.freqHz;
            tmpDet.time     = std::time(0);
            dets.push_back(tmpDet);
        }
    }
    return dets;
};
