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
#include "../../qtplot/qtplot.h"

namespace hackrf{
class proc{
  public:
    proc();
    ~proc();
    void init(int fftSize, int inputSize, int numBands,uint16_t startFreq); //init processors...filters and the like
    void rx_monitor(radar::charBuff* rx_buff); //wait for samples to come from the hardware
    void stop();
    
  private:
    void signal_int(); //handle possible iq imbalance, frequency tuning, image rejection, and signal detection
    void gui_handle(); //sends data to the gui
    
    
    //signal processing classes
    FFT* fftProc; 
    math* simdMath;
    console* log;
    qui* quiH;
    udpSender* udp;
    memBuff* memBuffer;
    std::vector<cfar*> cfarFilt;
    
    //threads
    std::thread detThread;
    std::mutex buffMutex;
    std::condition_variable waitForBuff;
    
    //variables
    std::vector<radar::cfloatIQ*> floatBuffs;
    std::vector<radar::cfloatIQ*> fftBuffs;
    std::vector<radar::floatIQ*> absBuffs;
    std::vector<radar::cfarDet> procDets;
    
    bool buffRdy,enabled,waiting,sweepStarted;
    
    
    int buffNum;
    int procNum;
    int buffLen;
    int numBuffs;
    int numBands;
    uint16_t startFreq;
    
  };
}
#endif
