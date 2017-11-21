#include "volk_math.h"

#include <iostream>
#include <cstring>
#include <volk/volk.h>

/*
 * Vectorized addition
 *
 */
void math::add(radar::complexFloat* input1, radar::complexFloat* input2, radar::complexFloat* output, int numSamps){
  volk_32f_x2_add_32f((float*)output,(float*)input1,(float*)input2,2*numSamps);
}

void math::add(float* input1 ,float* input2, float* output,int numSamps){
  volk_32f_x2_add_32f(output, input1, input2,numSamps);
}


/*
 * Vectorized normalize
 *
 */
void math::normalize(radar::complexFloat* input, float normConst,int numSamps){
  volk_32f_s32f_normalize((float*)input, normConst, 2*numSamps);
}

void math::normalize(float* input, float normConst,int numSamps){
  volk_32f_s32f_normalize(input, normConst, numSamps);
}

/*
 * Vectorized magnitude and magnitude squared
 *
 */

void math::abs(radar::complexFloat* input, float* output, int numSamps){
  volk_32fc_magnitude_32f(output, input, numSamps);
}

void math::magSqrd(radar::complexFloat* input, float* output, int numSamps){
  volk_32fc_magnitude_squared_32f(output, input, numSamps);
}

/*
 * Vectorized multiply
 *
 */
void math::multiply(radar::complexFloat* input1,radar::complexFloat* input2, radar::complexFloat* output, int numSamps){
  volk_32fc_x2_multiply_32fc(output,input1,input2,numSamps);
}

/*
 * Vectorized mean
 *
 */
void math::getMeanAndStdDev(float* input, float* mean,int numSamps){
  float stdDev = 0;
  volk_32f_stddev_and_mean_32f_x2(&stdDev,mean,input,numSamps);
}

/*
 * Vectorized linear to dB conversion
 *
 */
void math::lin2dB(float* input,float* output, int numSamps){
  //get the log of the input
  volk_32f_log2_32f(input,output,numSamps);
  //multiply by 10/log2(10)
  volk_32f_s32f_multiply_32f(output,output,10.0f/math::log2of10,numSamps);
}

/*
 * Convert uchar to complex float
 *
 */
void math::interleavedUCharToComplexFloat(radar::charBuff* input, radar::complexFloat* output, int numSamps){
    int_fast32_t i = 0;
    int_fast32_t j = 0;
    for(;j<numSamps;++j,i+=2){
        output[j] = radar::complexFloat(input[i],input[i+1])/128.f;
    }
}
