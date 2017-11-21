#include <iostream>
#include <vector>
#include <signal.h>
#include <fstream>

#include "../src/util/radarDataTypes.h"
#include "../src/util/logger/consoleLog.h"
#include "../src/signal_processing/fft.h"

#define PI 3.1459

void genTestSignal(radar::cfloatIQ* input, float freq, float sampRate);

int main(){
//     radar::cfloatIQ* testSig = new radar::cfloatIQ(10000);
//     radar::cfloatIQ* testFFT = new radar::cfloatIQ(1024);
//     radar::floatIQ*  testAbs = new radar::floatIQ(1024);
// 
//     genTestSignal(testSig,10000,100000);
// 
//     //get fft
//     FFT* fftProc = new FFT(1024,10000);
// 
//     fftProc->getFFT(testSig->iq,testFFT->iq);
// 
//     std::ofstream fftTest    = std::ofstream("fftTest.bin",std::ios::binary);
//     std::ofstream absTest    = std::ofstream("absTest.bin",std::ios::binary);
//     std::ofstream sigGenTest = std::ofstream("sigGenTest.bin",std::ios::binary);
// 
//     sigGenTest.write(reinterpret_cast<char*>(testSig->iq),2*10000*sizeof(float));
//     fftTest.write(reinterpret_cast<char*>(testFFT->iq),2*1024*sizeof(float));
//     absTest.write(reinterpret_cast<char*>(testAbs->iq),1024*sizeof(float));
// 
//     sigGenTest.close();
//     fftTest.close();
//     absTest.close();
// 
//     delete testAbs;
//     delete testSig;
//     delete testFFT;
}

void genTestSignal(radar::cfloatIQ* input, float freq, float sampRate){
    for(size_t i = 0;i<input->buffSize;++i){
        input->iq[i] = radar::complexFloat(cos(2*PI*i*freq/sampRate),sin(2*PI*i*freq/sampRate));
    }
};
