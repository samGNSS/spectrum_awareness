#ifndef __CFAR__
#define __CFAR__

#include <vector>

#include "../util/radarDataTypes.h"

#include "../util/math/volk_math.h"
#include "../util/logger/consoleLog.h"
#include "../udp/udpBase.h"


class cfar{
public:
  cfar(float alpha, int numCells, int numGuardBins, int buffLen, int numAvg);
  ~cfar();
  std::vector<radar::cfarDet> getDetections(radar::floatIQ* fftIn);
private:
  //private methods
  void filter();
  void findPeaks();
  
  float alpha;
  int numCells;
  int numGuardBins_;
  int buffLen;
  int numAvg;
  int avgCount;
  
  math* simdMath;
  console* log;
  udpSender* udp;
  
  float* forwardSlice;
  float* backwardsSlice;
  float* avgBuff;
  std::vector<radar::cfarDet> dets;
  
};

#endif
