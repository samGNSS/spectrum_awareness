#ifndef __FILTER_BASE__
#define __FILTER_BASE__

#include "../util/math/volk_math.h"

class filterIF{
public:
    virtual void init() = 0;
    virtual void set_taps() = 0;
    template<typename T> void process(T* input, T* output);

};

#endif
