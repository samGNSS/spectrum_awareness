#ifndef __HACKRF_PROC_H__
#define __HACKRF_PROC_H__

#include <atomic>
#include <fstream>
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "../../signal_processing/cfar.h"
#include "../../signal_processing/histogram.h"
#include "../../signal_processing/fft.h"
#include "driver/hackrf.h"
#include "../../util/radarDataTypes.h"
#include "../../util/math/volk_math.h"
#include "../../util/logger/consoleLog.h"
#include "../../util/buffer/memBuffer.h"
#include "../../udp/udpBase.h"

namespace hackrf {
class proc {
public:
proc();
~proc();
void init(sdr::detectorParams &detector,uint16_t startFreq);     //init processors...filters and the like
void rx_monitor(radar::charBuff* rx_buff);     //wait for samples to come from the hardware
void stop();

private:
void signal_int();     //handle possible iq imbalance, frequency tuning, image rejection, and signal detection
void gui_handle();     //sends data to the gui
void systemMonitor();


//signal processing classes
FFT* fftProc;
math* simdMath;
cfar* cfarFilt;

//utilities
console* log;
udpSender* udp;
memBuff* memBuffer;

//threads
std::thread detThread,monitorThread;
std::mutex buffMutex;
std::condition_variable waitForBuff;

//variables
std::vector<radar::cfloatIQ*> floatBuffs;
std::vector<radar::cfloatIQ*> fftBuffs;
std::vector<radar::floatIQ*> absBuffs;
std::vector<radar::cfarDet> procDets;

bool buffRdy,enabled,waiting,sweepStarted;

int procNum;
int buffLen;
int numBuffs;
int numDets;
uint16_t startFreq;

static constexpr int blocksPerTransfer = 16;     //I guess this is set in the firmware

};
}
#endif
