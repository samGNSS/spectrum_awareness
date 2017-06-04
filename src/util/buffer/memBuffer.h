#ifndef __MEM_BUFF__
#define __MEM_BUFF__

#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "../radarDataTypes.h"
#include "../logger/consoleLog.h"

class memBuff{
public:
    static memBuff* getInst();
    ~memBuff();
    
    void publishDets(std::vector<radar::cfarDet> dets);
    std::vector<radar::cfarDet> getDetsDataInRange(uint64_t startTime,uint64_t endTime);
    void ageOutDets(uint64_t startTime);
    void ageOutTimer();
    
    
private:
    static memBuff* inst;
    console* log;
    
    memBuff();
    
    std::map<uint64_t,radar::cfarDet> detMap;
    std::map<uint64_t,radar::cfarDet>::iterator detIt;
    
    std::chrono::seconds age;
    bool enabled;
    std::thread ageOut;
    std::mutex detMtx;
    std::condition_variable ageOutCv;
    
};


#endif
