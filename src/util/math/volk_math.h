//util for wrapping calls to volk functions
#ifndef __VOLK_MATH__
#define __VOLK_MATH__

#include "../radarDataTypes.h"

class math{
public:
    math(int numSamps);
    ~math();
    
    //normalization methods
    void normalize(radar::complexFloat* input,float normConst);
    void normalize(float* input, float normConst,int numSamps);
    void normalize(radar::complexFloat* input, float normConst,int numSamps);

    //addition methods
    void add(radar::complexFloat* input1,radar::complexFloat* input2,radar::complexFloat* output);
    void add(float* input1 ,float* input2, float* output,int numSamps);
    void add_const(radar::complexFloat* input,float addConst);
    
    //magnitude squared
    void magSqrd(radar::complexFloat* input,float* output);

    //multiplication
    void multiply(radar::complexFloat* input1,radar::complexFloat* input2, radar::complexFloat* output);
    void multiply(radar::complexFloat* input1,radar::complexFloat* input2, radar::complexFloat* output,int numSamps);

    //standard deviation and mean
    void getMeanAndStdDev(float* input, float* mean,int numSamps);

    //absolute value
    void abs(radar::complexFloat* input,float* output);

    //linear to dB
    void lin2dB(float* input,float* output);

    //uchar to complex float
    void interleavedUCharToComplexFloat(radar::charBuff* input,radar::complexFloat* output, int numSamps);
  
private:
    static constexpr radar::complexFloat one = radar::complexFloat(1,1);
    static constexpr float log2of10 = 3.3219280948874;
    unsigned int alignment;//memory alignment for volk
    int buffSize;

    float* internalFloatBuff; //for kernels that require a float input
};
#endif
