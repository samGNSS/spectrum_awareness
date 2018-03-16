#ifndef __FILTER_BASE__
#define __FILTER_BASE__

#include <complex>
#include <fftw3.h>
#include <string>

#include "../util/math/volk_math.h"
#include "../util/math/volk_math.h"

class filterIF{
public:
    void init(int fftSize, int kernelSize);
    template<typename T> void process(T* input, T* output);

private:
    //initialize fftw
    void initFFT();
    void cleanUpFFT();

    fftwf_plan forwardDFT;
    fftwf_plan reverseDFT;

    int m_kernelSize;
    int m_fftSize;


};

#endif
