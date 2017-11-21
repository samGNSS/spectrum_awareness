#include "timing.h"

timer::timer(){
    startTime = std::chrono::high_resolution_clock::now();
}

timer::~timer(){}

std::pair<uint, uint> timer::getTime(){
    auto timeDiff = std::chrono::high_resolution_clock::now() - startTime;
    double time = timeDiff.count();
    return std::pair<uint,uint>((uint)time,(uint)(time * 1e6));
}


