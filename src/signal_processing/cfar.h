#include <queue>
#include <vector>

#include "../util/radarDataTypes.h"

#include "../util/math/volk_math.h"
#include "../util/logger/consoleLog.h"

class cfar{
public:
  cfar(std::queue<std::vector<radar::cfarDet>>* quiQueue,float alpha,int numCells,int numGuardBins,int buffLen);
  ~cfar();
  void getDetections(radar::floatIQ* fftIn);
private:
  //private methods
  void filter();
  void findPeaks();
  
  float alpha;
  int numCells;
  int numGuardBins_;
  int buffLen;
  
  math* simdMath;
  console* log;
  
  float* forwardSlice;
  float* backwardsSlice;
  std::vector<radar::cfarDet> dets;
  std::queue<std::vector<radar::cfarDet>>* quiQueue;
};