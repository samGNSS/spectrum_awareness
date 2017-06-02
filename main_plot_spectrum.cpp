#include <iostream>
#include <vector>
#include <signal.h> 

#include <boost/program_options.hpp>

#include "src/hardware/hackrf/driver/device_setup.h"
#include "src/util/radarDataTypes.h"
#include "src/util/logger/consoleLog.h"
#include "src/hardware/hackrf/scheduler.h"
#include "src/qtplot/qtWindow.h"
#include "src/udp/udpBase.h"


volatile sig_atomic_t flag = 1;

void handleInterrupt(int sig){
    flag = 0;
}

namespace po = boost::program_options;
using namespace radar;
int main(int argc, char **argv) {
    
    signal(SIGINT, handleInterrupt); 
    
    //variables to be set by po
    std::string args;
    double duration;
    uint32_t rate,filterBw,rxVgaGain,rxLnaGain,txVgaGain;
    uint64_t centerFreq;
    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args",            po::value<std::string>(&args)->default_value(""), "single device address args")
        ("duration",        po::value<double>(&duration)->default_value(0.0001), "duration for the tx waveform in seconds")
        ("rate",            po::value<uint32_t>(&rate)->default_value(20e6), "sample rate (sps)")
	("baseBandFilerBw", po::value<uint32_t>(&filterBw)->default_value(15e6), "baseband filter bandwidth (Hz)")
	("rxVgaGain",       po::value<uint32_t>(&rxVgaGain)->default_value(20), "rx gain")
	("rxLnaGain",       po::value<uint32_t>(&rxLnaGain)->default_value(16), "rx lna gain")
	("txVgaGain",       po::value<uint32_t>(&txVgaGain)->default_value(0), "tx gain")
	("centerFreq",      po::value<uint64_t>(&centerFreq)->default_value(2437e6), "center frequency (Hz)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    const sdr::device_params frontEnd = sdr::device_params{centerFreq,
							    rate,
							    filterBw,
							    rxVgaGain,
							    rxLnaGain,
							    txVgaGain};
 
    std::vector<radar::freqRange> bands(1); 
    
    float freq = 88.7e6;
    float bw = 20e6;
    for(uint i = 0;i<bands.size();++i){
      bands[i].first = freq;
      bands[i].second = freq+bw;
      freq += 2*bw;
    }
    
    udpSender* udp = udpSender::getInstance();
    udp->init(1234);   
    
    //start spectrum monitor
    hackrf::sched* specSched = new hackrf::sched(&frontEnd);
    specSched->findDevices();
    specSched->init(bands,0.0001);
    specSched->start(); 
       
    while(flag){usleep(10000);};
    
    delete specSched;  
    return 0;
}
