#include "volk_math.h"

#include <iostream>
#include <cstring>
#include <volk/volk.h>

#define log2of10 3.3219280948874 

//TODO:: make this a sigleton so that there aren't 15 different copies of this class all over the place. The only issue is the filtering stuff, but that can probably be removed

math::math(int numSamps):buffSize(numSamps){  
  alignment = volk_get_alignment();
  filterInit = false;
};

math::~math(){
  if(filterInit){
    volk_free(internalFilterHistory);
    volk_free(aligned_taps);
    volk_free(complexZeros);
  }
};

void math::add(radar::complexFloat* input1, radar::complexFloat* input2, radar::complexFloat* output){
  volk_32f_x2_add_32f((float*)output,(float*)input1,(float*)input2,2*buffSize);
}


void math::normalize(radar::complexFloat* input, float normConst){
  volk_32f_s32f_normalize((float*)input, normConst, 2*buffSize);
}

void math::abs(radar::complexFloat* input,float* output){
  volk_32fc_magnitude_32f(output, input, buffSize);
}

void math::magSqrd(radar::complexFloat* input,float* output){
  volk_32fc_magnitude_squared_32f(output, input, buffSize);
}

void math::multiply(radar::complexFloat* input1,radar::complexFloat* input2, radar::complexFloat* output){
  volk_32fc_x2_multiply_32fc(output,input1,input2,buffSize);
}

void math::multiply(radar::complexFloat* input1,radar::complexFloat* input2, radar::complexFloat* output,int numSamps){
  volk_32fc_x2_multiply_32fc(output,input1,input2,numSamps);
}

void math::getMeanAndStdDev(float* input, float* mean,int numSamps){
  float stdDev = 0;
  volk_32f_stddev_and_mean_32f_x2(&stdDev,mean,input,numSamps);
}

void math::lin2dB(float* input,float* output){
  //get the log of the input
  volk_32f_log2_32f(input,output,buffSize);
  //divide log2(10) to get log10
  volk_32f_s32f_normalize(output,log2of10,buffSize);
  //multiply by 10
  volk_32f_s32f_multiply_32f(output,output,10.0f,buffSize);
}

void math::initFilter(float* taps, int tapsSize){
  if(!filterInit){
    filterInit = true;
    numTaps = tapsSize;
    //set up filter history buffer
    internalFilterHistory = (radar::complexFloat*)volk_malloc(tapsSize*sizeof(radar::complexFloat),alignment);
    complexZeros = (radar::complexFloat*)volk_malloc(tapsSize*sizeof(radar::complexFloat),alignment);

    for(int i = 0;i<tapsSize;i++){
      internalFilterHistory[i] = radar::complexFloat(0,0);
      complexZeros[i] = radar::complexFloat(0,0);
    }
    
    //store filter taps
    aligned_taps = (float*)volk_malloc(tapsSize*sizeof(float),alignment);
    std::memcpy(aligned_taps,taps,tapsSize*sizeof(float));
  }else{
    //new filter
    if(numTaps==tapsSize){
      std::memcpy(aligned_taps,taps,tapsSize*sizeof(float));
    }
    else{
      numTaps = tapsSize;
      volk_free(aligned_taps);
      aligned_taps = (float*)volk_malloc(tapsSize*sizeof(float),alignment);
      std::memcpy(aligned_taps,taps,tapsSize*sizeof(float));
    }
  }
}

void math::filter(radar::complexFloat* input, radar::complexFloat* output){
  //linear covolution
  for(int i = 0;i<buffSize;++i){
    //run through accumelator
    internalFilterHistory[0] = input[i];
    volk_32fc_32f_dot_prod_32fc(&output[i],internalFilterHistory,aligned_taps,numTaps);
    
    //roll filter history
    std::memmove(&internalFilterHistory[1],&internalFilterHistory[0],(numTaps-1)*sizeof(radar::complexFloat));
  }
  //done filtering, zero out the history. This is done because the inputs to this function are not time aligned
  std::memcpy(internalFilterHistory,complexZeros,(numTaps)*sizeof(radar::complexFloat));
}