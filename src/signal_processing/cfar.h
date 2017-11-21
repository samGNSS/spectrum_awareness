#ifndef __CFAR__
#define __CFAR__

#include <fstream>
#include <vector>

#include "../util/radarDataTypes.h"

#include "../util/math/volk_math.h"
#include "../util/logger/consoleLog.h"
#include "../udp/udpBase.h"
#include "../util/timing/timing.h"

class cfar{
public:
  cfar(float pfa, int numCells, int numGuardBins, int buffLen, int freqSlip, int sampleRate);
  ~cfar();
  std::vector<radar::target> getDetections(radar::floatIQ* fftIn);
private:
  //private methods
  std::vector<radar::target> compaction(std::vector<radar::cfarDet> dets);
  radar::target newTarget(radar::cfarDet det);
  radar::target addDetToTarget(radar::target target,radar::cfarDet det);

  void InitTrainingCells(float* input);

  float m_alpha;
  float m_freqReso;
  int m_numCells;
  int m_numGuardBins;
  int m_buffLen;
  int m_freqSlip;
  int m_freqSlipHz;
  int m_sampleRate;

  timer* time;

  float m_forwardNoiseEstimate;
  float m_reverseNoiseEstimate;

  size_t m_forwardStart,m_forwardEnd;
  size_t m_reverseStart,m_reverseEnd;

  std::ofstream dbgFile;

};

#endif
