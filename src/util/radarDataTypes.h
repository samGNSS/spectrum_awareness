//typedefs

#ifndef __radar_data__
#define __radar_data__

#include <complex>
#include <memory>
#include <vector>
#include <volk/volk.h>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdint.h>

//IQ/Detection data types
namespace radar{
    //base types
    typedef std::complex<float> complexFloat;
    typedef int8_t charBuff;
    typedef std::shared_ptr<complexFloat> complexFloatBuffPtr;
    typedef std::shared_ptr<charBuff> charBuffPtr;
    typedef std::pair<uint64_t,uint32_t> centerFreqAndBW;
    typedef std::pair<uint64_t,uint64_t> freqRange;


    typedef struct{
      uint64_t time;
      uint64_t freqHz;
      uint8_t band;
      uint32_t sampRate;
      bool valid;
    }iqMd;

    class charIQ{
    public:
      charIQ(uint_fast64_t buffSize_):buffSize(buffSize_){iq = (charBuff*)volk_malloc(buffSize*sizeof(charBuff),volk_get_alignment());};
      ~charIQ(){volk_free(iq);};
      charBuff* iq;
      uint_fast64_t buffSize;
      iqMd metaData;
    };

    class cfloatIQ{
    public:
      cfloatIQ(uint_fast64_t buffSize_):buffSize(buffSize_){iq = (complexFloat*)volk_malloc(buffSize*sizeof(complexFloat),volk_get_alignment());};
      ~cfloatIQ(){volk_free(iq);};
      complexFloat* iq;
      uint_fast64_t buffSize;
      iqMd metaData;
    };

    class floatIQ{
    public:
      floatIQ(uint_fast64_t buffSize_):buffSize(buffSize_){iq = (float*)volk_malloc(buffSize*sizeof(float),volk_get_alignment());};
      ~floatIQ(){volk_free(iq);};
      float* iq;
      uint_fast64_t buffSize;
      iqMd metaData;
    };

#pragma pack(push,1)
    //processed detection
    typedef struct{
      uint64_t timeOn;
      uint64_t timeOnMicroSec;
      uint64_t freqHz;
      double   power;
    }cfarDet;

    //Target
    typedef struct{
      uint64_t timeOn;
      uint64_t timeOnMicroSec;
      centerFreqAndBW freqHz;
      double   power;
    }target;

    //histogram channel
    typedef struct{
      uint32_t startFreq;
      uint32_t stopFreq;
      uint64_t timeOn;
      uint64_t timeOff;
      double   confidence;
    }channel;
#pragma pack(pop)
};


//hardware data types
namespace sdr{
    typedef struct{
        uint64_t centerFreq;
        uint32_t sampRate;
        uint32_t baseBandFiltBw;
        uint32_t rxVgaGain;
        uint32_t rxLnaGain;
        uint32_t txVgaGain;
    }deviceParams;

    typedef struct{
        uint64_t startFreq;
        int bandwidth;
        int numBands;
        float dwellTime;
        bool enabled;
    }scannerParams;

    typedef struct{
        int fftSize;
        int freqSlip;
        float pfa;
        int numGuardBins;
        int numAvgBins;
        std::string windowType;
    }detectorParams;

};
#endif
