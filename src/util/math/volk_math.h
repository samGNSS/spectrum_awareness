//util for wrapping calls to volk functions
#ifndef __VOLK_MATH__
#define __VOLK_MATH__

#include "../radarDataTypes.h"

class math{
public:
    math(int numSamps);
    ~math();
    void normalize(radar::complexFloat* input,float normConst);
    void normalize(float* input, float normConst,int numSamps);

    void add(radar::complexFloat* input1,radar::complexFloat* input2,radar::complexFloat* output);
    void add(float* input1 ,float* input2, float* output,int numSamps);

    void runningAverage(radar::complexFloat* input1,radar::complexFloat* input2,radar::complexFloat* output);

    void magSqrd(radar::complexFloat* input,float* output);

    void add_const(radar::complexFloat* input,float addConst);

    void multiply(radar::complexFloat* input1,radar::complexFloat* input2, radar::complexFloat* output);
    void multiply(radar::complexFloat* input1,radar::complexFloat* input2, radar::complexFloat* output,int numSamps);

    void getMeanAndStdDev(float* input, float* mean,int numSamps);

    void abs(radar::complexFloat* input,float* output);

    void lin2dB(float* input,float* output);

    void interleavedUCharToComplexFloat(radar::charBuff* input,radar::complexFloat* output, int numSamps);
  
private:
    static constexpr radar::complexFloat one = radar::complexFloat(1,1);
    static constexpr float log2of10 = 3.3219280948874;
    unsigned int alignment;//memory alignment for volk
    int buffSize;

    float* internalFloatBuff; //for kernels that require a float input
};
#endif
