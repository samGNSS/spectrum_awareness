#ifndef __TIMING__
#define __TIMING__

#include <chrono>
#include <algorithm>

class timer{
    public:
        timer();
        ~timer();
        std::pair<uint,uint> getTime();
        
    private:
        std::chrono::high_resolution_clock::time_point startTime;
};

#endif
