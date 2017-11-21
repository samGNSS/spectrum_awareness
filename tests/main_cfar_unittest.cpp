#include <iostream>
#include <fstream>
#include <chrono>
#include <complex>
#include <vector>
#include <string>

#include "../src/signal_processing/cfar.h"
#include "../src/signal_processing/fft.h"
#include "../src/util/logger/consoleLog.h"
#include "../src/util/radarDataTypes.h"

//util
template<typename T> void writeFFT(T* data, int numPoints, std::string fname){
    std::ofstream out = std::ofstream(fname,std::ios::binary);
    out.write(reinterpret_cast<char*>(data),numPoints*sizeof(T));
    out.close();
}

//Tests
bool test1(cfar* processor,int fftSize){
    radar::cfloatIQ signal = radar::cfloatIQ(fftSize);
    radar::floatIQ signalAbs = radar::floatIQ(fftSize);

    for(size_t i = 0; i < (size_t)fftSize; ++i){
        signal.iq[i] = std::complex<float>(std::cos((float)2*M_PI*1500*i/10000),std::sin((float)2*M_PI*1500*i/10000));
        signal.iq[i] += std::complex<float>(std::cos((float)2*M_PI*1000*i/10000),std::sin((float)2*M_PI*1000*i/10000));
        signal.iq[i] += std::complex<float>(std::cos((float)2*M_PI*1100*i/10000),std::sin((float)2*M_PI*1100*i/10000));
        signal.iq[i] += std::complex<float>(std::cos((float)2*M_PI*1200*i/10000),std::sin((float)2*M_PI*1200*i/10000));
        signal.iq[i] += std::complex<float>(std::cos((float)2*M_PI*1300*i/10000),std::sin((float)2*M_PI*1300*i/10000));
    }

    writeFFT<std::complex<float>>(signal.iq, 1024, "cfar_test_fft.bin");

    //get fft
    FFT fftProc = FFT(fftSize,fftSize,"blackman");
    fftProc.getFFT_ABS(signal.iq,signalAbs.iq);
    signalAbs.metaData.freqHz = 2400e6;

    // for(size_t i = 0; i < fftSize; ++i){
    //     signalAbs.iq[i] += 0.02;
    // }

    writeFFT<float>(signalAbs.iq, 1024, "cfar_test_fft_mag.bin");

    //run through cfar
    std::vector<radar::target> targets = processor->getDetections(&signalAbs);

    std::cout << "Number of targets: " << targets.size() << std::endl;

    for(auto target : targets){
        std::cout << target.freqHz.first << ", " << target.freqHz.second << std::endl;
    }

    return true;
}


int main(){

    console::info(__FILENAME__, __LINE__, "Running CFAR unit test");
    cfar* processor = new cfar(0.001,5,2,1024,5,10000);
    test1(processor,1024);

    delete processor;
    return 1;
}
