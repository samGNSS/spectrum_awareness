#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "src/hardware/hackrf/driver/device_setup.h"
#include "src/util/radarDataTypes.h"
#include "src/util/logger/consoleLog.h"
#include "src/hardware/hackrf/scheduler.h"
#include "src/qtplot/qtWindow.h"

namespace po = boost::program_options;
using namespace radar;
int main(int argc, char **argv) {
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
	("baseBandFilerBw", po::value<uint32_t>(&filterBw)->default_value(rate/2), "baseband filter bandwidth (Hz)")
	("rxVgaGain",       po::value<uint32_t>(&rxVgaGain)->default_value(8), "rx gain")
	("rxLnaGain",       po::value<uint32_t>(&rxLnaGain)->default_value(8), "rx lna gain")
	("txVgaGain",       po::value<uint32_t>(&txVgaGain)->default_value(32), "tx gain")
	("centerFreq",      po::value<uint64_t>(&centerFreq)->default_value(94.3e6), "center frequency (Hz)")
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
 
    std::vector<radar::freqRange> bands(5); 
    
    float freq = 2400e6;
    float bw = 20e6;
    for(uint i = 0;i<bands.size();++i){
      bands[i].first = freq;
      bands[i].second = freq+bw;
      freq += 2*bw;
    }
    
    //set up plot window before starting the spectrum monitor
    QApplication a(argc, argv);

    qtWindow mainWindow;
#if QT_VERSION < 0x040000
    a.setMainWidget(&mainWindow);
#endif

    mainWindow.resize(600,400);
    mainWindow.show();
    
    //start spectrum monitor
    hackrf::sched* specSched = new hackrf::sched(&frontEnd);
    specSched->findDevices();
    specSched->init(bands,0.001);
    specSched->start(); 

    //block until the window is closed
    a.exec();
//     usleep(3000000);
    
    delete specSched;  
    return 0;
}