#ifndef __CONFIG_PARSER__
#define __CONFIG_PARSER__

#include <string>

#include "../radarDataTypes.h"

#include "rapidxml-1.13/rapidxml.hpp"

class configParser{
public:
    configParser(std::string configFileName);
    ~configParser();
     std::string getDatabaseName();
     void getRadioParams(sdr::deviceParams& frontEnd);
     void getScannerParams(sdr::scannerParams& scanner);
     void getDetectorParams(sdr::detectorParams& detector);
     
private:
    sdr::deviceParams _frontEnd;
    sdr::scannerParams _scanner;
    sdr::detectorParams _detector;
    
    std::string _dbName;
    
    void parseRadio(rapidxml::xml_node<> *root);
    void parseScanner(rapidxml::xml_node<> *root);
    void parseDetector(rapidxml::xml_node<> *root);
    
    rapidxml::xml_document<> _doc;
	rapidxml::xml_node<> * _rootNode;
};

#endif
