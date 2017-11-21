#ifndef __BASEPROC__
#define __BASEPROC__

#include <vector>

#include "../../util/radarDataTypes.h"

class baseProcessor{
public:
  //baseSched();
  virtual ~baseProcessor(){};
  virtual void Init(sdr::detectorParams &detector, uint16_t startFreq, uint16_t endFreq) = 0;
  virtual void Stop() = 0;
};

#endif
