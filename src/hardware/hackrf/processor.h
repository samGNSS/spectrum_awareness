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
#include "../HardwareBase/baseProc.h"

#define debug
#ifdef debug
#include <fstream>
#endif

namespace hackrf {
class processor : baseProcessor {
public:
    processor();
    ~processor();
    void Init(sdr::detectorParams &detector, uint16_t startFreq, uint16_t endFreq);
    void RxMonitor(radar::charBuff* rx_buff);     //wait for samples to come from the hardware
    void Stop();

private:
void SignalInt();     //handle possible iq imbalance, frequency tuning, image rejection, and signal detection
void SystemMonitor();

#ifdef debug
std::ofstream dbgFile;
// template<typename T> void writeFFT(std::ofstream& ofile, int32_t numSamps, T* fftIn){
//     ofile.write(reinterpret_cast<const char*>(numSamps), sizeof(int32_t));
//     ofile.write(reinterpret_cast<const char*>(fftIn), numSamps*sizeof(T));
// }
#endif

//signal processing classes
FFT*  m_fftProc;
cfar* m_cfarFilt;

//utilities
// udpSender* udp;
// memBuff* memBuffer;

//threads
std::thread m_detThread, m_monitorThread;
std::mutex m_buffMutex;
std::condition_variable m_waitForBuff;

//variables
std::vector<radar::cfloatIQ*> m_floatBuffs;
std::vector<radar::cfloatIQ*> m_fftBuffs;
std::vector<radar::floatIQ*> m_absBuffs;
std::vector<radar::target> m_procDets;

bool m_buffRdy, m_enabled, m_waiting, m_sweepStarted;

int m_procNum, m_buffLen, m_numBuffs, m_numDets;
uint16_t m_startFreq, m_endFreq;

static constexpr int m_blocksPerTransfer = 16;     //I guess this is set in the firmware

};
}
#endif
