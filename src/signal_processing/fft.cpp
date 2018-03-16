#include "fft.h"

#include <volk/volk.h>

FFT::FFT(int fftSize,int inputSize,std::string windowType):fftSize(fftSize){
  outputMem = fftwf_alloc_complex(fftSize*sizeof(fftwf_complex));
  tmp = fftwf_alloc_complex(inputSize*sizeof(fftwf_complex));
  forwardDFT = fftwf_plan_dft_1d(fftSize,tmp,outputMem,FFTW_FORWARD,FFTW_MEASURE);
  inverseDFT = fftwf_plan_dft_1d(fftSize,tmp,outputMem,FFTW_BACKWARD,FFTW_MEASURE);

  //define window
  window = (radar::complexFloat*)volk_malloc(inputSize*sizeof(radar::complexFloat),volk_get_alignment());
  this->setWindow(windowType,inputSize);
};

FFT::~FFT(){
  volk_free(window);

  //free tmp
  fftwf_free(tmp);
  fftwf_free(outputMem);

  fftwf_destroy_plan(forwardDFT);
  fftwf_destroy_plan(inverseDFT);
};

void FFT::resetFFTSize(int fftSize,int inputSize){
  this->fftSize = fftSize;
  outputMem  = fftwf_alloc_complex(fftSize*sizeof(fftwf_complex));
  tmp        = fftwf_alloc_complex(inputSize*sizeof(fftwf_complex));
  forwardDFT = fftwf_plan_dft_1d(fftSize,tmp,outputMem,FFTW_FORWARD,FFTW_ESTIMATE);
  inverseDFT = fftwf_plan_dft_1d(fftSize,tmp,outputMem,FFTW_BACKWARD,FFTW_ESTIMATE);

};

void FFT::getFFT(radar::complexFloat* input, radar::complexFloat* output){
  math::multiply(input,window, input, fftSize);
  fftwf_complex* inputTMP  = reinterpret_cast<fftwf_complex*>(input);
  fftwf_complex* outputTMP = reinterpret_cast<fftwf_complex*>(output);
  fftwf_execute_dft(forwardDFT, inputTMP, outputTMP);
  math::normalize(output, fftSize, fftSize);
};


void FFT::getFFT_ABS(radar::complexFloat* input, float* output){
  math::multiply(input, window, input, fftSize);
  std::memmove(this->tmp, input, sizeof(fftwf_complex)*this->fftSize);
  fftwf_execute_dft(forwardDFT, this->tmp,this->outputMem);
  radar::complexFloat* out = reinterpret_cast<radar::complexFloat*>(this->outputMem);
  math::normalize(out, (float)fftSize, fftSize);
  math::magSqrd(out, output, fftSize);
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
    }else if(windowType == "blackman"){
        for(int i=0;i<windowSize;++i){
            window[i] = 0.42 - 0.5*std::cos(2*M_PI*i/(fftSize-1)) + 0.08*std::cos(4*M_PI*i/(fftSize-1));
        }
    }
};
