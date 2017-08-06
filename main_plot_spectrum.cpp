#include <iostream>
#include <vector>
#include <signal.h> 

#include <boost/program_options.hpp>

#include "src/hardware/hackrf/driver/device_setup.h"
#include "src/util/radarDataTypes.h"
#include "src/util/logger/consoleLog.h"
#include "src/util/logger/databaseLogger.h"
#include "src/util/config_parser/configParser.h"
#include "src/hardware/hackrf/scheduler.h"
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
    std::string configFile;
    
    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Monitor frequencies")
        ("configFile",      po::value<std::string>(&configFile)->default_value("../configs/default.xml"), "xml configuration file");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {  
        std::cout << desc << "\n";
        return 0;
    }
    
    sdr::deviceParams frontEnd;
    sdr::scannerParams scanner;
    sdr::detectorParams detector;
    
    configParser *parser = new configParser(configFile);
    
    parser->getDetectorParams(detector);
    parser->getScannerParams(scanner);
    parser->getRadioParams(frontEnd);
    
    //const sdr::deviceParams quick = frontEnd;
     
    //init database
    //databaseLogger* main_db_inst = databaseLogger::getInst(dBName);
    
//     //init udp sender
    udpSender* udp = udpSender::getInstance();
    udp->init(1234);   
//     
//     //start spectrum monitor
    hackrf::sched* specSched = new hackrf::sched();
    specSched->findDevices();
    specSched->init(frontEnd,scanner,detector);
    specSched->start(); 
    
//     
//     //run the system
    while(flag){usleep(10000);};
//     
//     //clean up memory, the order here matters: delete the schedule first
    delete specSched;  
//     //delete main_db_inst;
    return 0;
}
