#ifndef __MEM_BUFF__
#define __MEM_BUFF__

#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <boost/circular_buffer.hpp>

#include "../radarDataTypes.h"
#include "../logger/consoleLog.h"
#include "../logger/databaseLogger.h"

class memBuff{
public:
    ~memBuff();
    memBuff();

private:
    boost::circular_buffer<radar::target> targetBuffer;
};


#endif
