#ifndef __HIST__
#define __HIST__

#include <vector>

#include "../util/radarDataTypes.h"

class histogram{
public:
    histogram();
    ~histogram();
    std::vector<int> makeChannels(std::vector<radar::cfarDet> dets);
private:
    void getBins(std::vector<radar::cfarDet> dets);
    void getThresh();
    void resizeBin();
};

#endif
