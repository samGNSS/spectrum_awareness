#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <vector>

#include "../util/radarDataTypes.h"
#include "../util/logger/consoleLog.h"

class histogram{
public:
    histogram(int numBands, int bandWidth, double threshold, double age);
    ~histogram();
    std::vector<int> makeChannels(std::vector<radar::cfarDet> dets);
private:
    void getBins(std::vector<radar::cfarDet> dets);
    void getThresh();
    
    
    console* log;
};

#endif
