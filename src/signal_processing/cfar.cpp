#include "cfar.h"

#include <cstring>
#include <volk/volk.h>
#include <iostream>
#include <vector>


cfar::cfar(std::queue<std::vector<radar::cfarDet>>* quiQueue,float alpha,int numCells,int numGuardBins,int buffLen):alpha(alpha),
                                                     numCells(numCells/2),
                                                     numGuardBins_(numGuardBins),
                                                     buffLen(buffLen),quiQueue(quiQueue)
{
  forwardSlice = (float*)volk_malloc((this->numCells)*sizeof(float),volk_get_alignment());
  backwardsSlice = (float*)volk_malloc((this->numCells)*sizeof(float),volk_get_alignment());
  dets.reserve(1024);  
  simdMath = new math(this->numCells);
  log = new console();
}
  
cfar::~cfar(){
  volk_free(forwardSlice);
  volk_free(backwardsSlice);
  dets.clear();
  
  delete simdMath;
  delete log;
};

void cfar::getDetections(radar::floatIQ* fftIn){
  //CFAR processing
  //First pass at an implementation, its very verbose and could probably be improved a lot
  int bin = 0;
  int cell = 0;
  float forwardMean = 0;
  float backwardsMean = 0;
  float noiseEsti = 0;
  int detCount = 0;
  
  radar::cfarDet tmpDet;

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
        tmpDet.startBin = bin;
	tmpDet.stopBin  = bin;
	tmpDet.power    = fftIn->iq[bin];
	tmpDet.freqHz   = fftIn->metaData.freqHz;
	tmpDet.time     = fftIn->metaData.time;
      dets.push_back(tmpDet);
      ++detCount;
    }
  }
  
  log->debug(__FILENAME__,__LINE__, "Number of detections: %d \t noiseEsti: %f",detCount,noiseEsti);
  
  quiQueue->emplace(dets);
  
  dets.clear();  
};

// void initDet(radar::cfarDet& det){
//   det.startBin = 0;
//   det.stopBin  = 0;
//   det.power    = 0;
//   det.freqHz   = 0;
//   det.time     = 0;
// }
