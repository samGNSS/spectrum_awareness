#ifndef __BASESCHED__
#define __BASESCHED__

#include <vector>

#include "../../util/radarDataTypes.h"

class baseSched{
public:
  //baseSched();
  virtual ~baseSched(){};
  virtual void findDevices() = 0;
  virtual void init(std::vector<radar::freqRange> scanBands, float dwellTime) = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
};

#endif