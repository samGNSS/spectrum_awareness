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

//file name macro
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

//IQ/Detection data types
namespace radar{
    //base types
    typedef std::complex<float> complexFloat;
    typedef uint8_t charBuff;
    typedef std::shared_ptr<complexFloat> complexFloatBuffPtr;
    typedef std::shared_ptr<charBuff> charBuffPtr;
    typedef std::pair<uint64_t,uint32_t> centerFreqAndBW;
    typedef std::pair<uint64_t,uint64_t> freqRange;
        
    
    typedef struct{
      uint64_t time;
      uint64_t freqHz;
      bool valid; 
    }iqMd;
    
    class charIQ{
    public:
      charIQ(int_fast64_t buffSize_):buffSize(buffSize_){iq = (charBuff*)volk_malloc(buffSize*sizeof(charBuff),volk_get_alignment());};
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
    
    //processed detection
    typedef struct{
      int startBin;
      int stopBin;
      double power;
      uint64_t time;
      uint64_t freqHz;
    }cfarDet;    
      
    //processed detection
    typedef struct{
      freqRange range;
      double power;
      uint64_t time;
    }det;
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
  }device_params;
};
#endif
