#include "fft.h"

#include <volk/volk.h>

FFT::FFT(int fftSize,int inputSize,std::string windowType):fftSize(fftSize){
  outputMem = fftwf_alloc_complex(fftSize*sizeof(fftwf_complex));
  tmp = fftwf_alloc_complex(inputSize*sizeof(fftwf_complex));
  forwardDFT = fftwf_plan_dft_1d(fftSize,tmp,outputMem,FFTW_FORWARD,FFTW_ESTIMATE); 
  inverseDFT = fftwf_plan_dft_1d(fftSize,tmp,outputMem,FFTW_BACKWARD,FFTW_ESTIMATE);
  
  //define window
  window = (radar::complexFloat*)volk_malloc(inputSize*sizeof(radar::complexFloat),volk_get_alignment());
  this->setWindow(windowType,inputSize);
  //math
  simdMath = new math(inputSize);
  
  //free tmp
  fftwf_free(tmp);
  fftwf_free(outputMem);
};

FFT::~FFT(){
  volk_free(window);
  fftwf_destroy_plan(forwardDFT);
  fftwf_destroy_plan(inverseDFT);
  delete simdMath;
};

void FFT::resetFFTSize(int fftSize,int inputSize){
  this->fftSize = fftSize;
  outputMem = fftwf_alloc_complex(fftSize*sizeof(fftwf_complex));
  tmp = fftwf_alloc_complex(inputSize*sizeof(fftwf_complex));
  forwardDFT = fftwf_plan_dft_1d(fftSize,tmp,outputMem,FFTW_FORWARD,FFTW_ESTIMATE); 
  inverseDFT = fftwf_plan_dft_1d(fftSize,tmp,outputMem,FFTW_BACKWARD,FFTW_ESTIMATE);
  
  //free tmp
  fftwf_free(tmp);
  fftwf_free(outputMem);
};

void FFT::getFFT(radar::complexFloat* input, radar::complexFloat* output){
  simdMath->multiply(input,window,input);
  fftwf_complex* inputTMP = reinterpret_cast<fftwf_complex*>(input);
  fftwf_complex* outputTMP = reinterpret_cast<fftwf_complex*>(output);
  fftwf_execute_dft(forwardDFT,inputTMP,outputTMP);
  simdMath->normalize(output,fftSize,fftSize);
};


void FFT::getFFT_ABS(radar::complexFloat* input, radar::complexFloat* output){
  simdMath->multiply(input,window,input);
  fftwf_complex* inputTMP = reinterpret_cast<fftwf_complex*>(input);
  fftwf_complex* outputTMP = reinterpret_cast<fftwf_complex*>(output);
  fftwf_execute_dft(forwardDFT,inputTMP,outputTMP);
  simdMath->normalize(output,fftSize,fftSize);
};


void FFT::getIFFT(radar::complexFloat* input, radar::complexFloat* output){
  fftwf_complex* inputTMP = reinterpret_cast<fftwf_complex*>(input);
  fftwf_complex* outputTMP = reinterpret_cast<fftwf_complex*>(output);
  fftwf_execute_dft(inverseDFT,inputTMP,outputTMP);
};

void FFT::setWindow(std::string windowType,int windowSize){
    //Hanning window
    if(windowType == "hanning"){
        for(int i=0;i<windowSize;++i){
            window[i] = 0.5f*(1.f - (float)std::cos(2*M_PI*i/(fftSize-1)));
        }
    }
};
