#include "configParser.h"
#include <fstream>
#include <iostream>
#include <vector>

configParser::configParser(std::string configFileName){
    std::ifstream fileReader(configFileName);
    std::vector<char> buffer((std::istreambuf_iterator<char>(fileReader)), std::istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc
	_doc.parse<0>(&buffer[0]);

    _rootNode =_doc.first_node("monitor");
    _dbName = _rootNode->first_attribute("dbName")->value();

    parseDetector(_rootNode);
    parseRadio(_rootNode);
    parseScanner(_rootNode);

}

configParser::~configParser(){}

void configParser::parseDetector(rapidxml::xml_node<>* root){
    rapidxml::xml_node<>* detectorNode = root->first_node("detector");
    _detector.fftSize      = std::stod(detectorNode->first_node("fft_size")->value());
    _detector.freqSlip     = std::stod(detectorNode->first_node("freq_slip")->value());
    _detector.pfa          = std::stof(detectorNode->first_node("pfa")->value());
    _detector.numAvgBins   = std::stod(detectorNode->first_node("num_avg_bins")->value());
    _detector.numGuardBins = std::stod(detectorNode->first_node("num_guard_bins")->value());
    _detector.windowType   = detectorNode->first_attribute("window")->value();

}

void configParser::parseRadio(rapidxml::xml_node<>* root){
    rapidxml::xml_node<>* radioNode = root->first_node("radio");
    _frontEnd.baseBandFiltBw = std::stoul(radioNode->first_node("base_band_bandwidth")->value());
    _frontEnd.centerFreq     = std::stoul(radioNode->first_node("center_frequency")->value());
    _frontEnd.rxLnaGain      = std::stoul(radioNode->first_node("lna_gain")->value());
    _frontEnd.rxVgaGain      = std::stoul(radioNode->first_node("vga_gain")->value());
    _frontEnd.sampRate       = std::stoul(radioNode->first_node("sample_rate")->value());
    _frontEnd.txVgaGain      = 0;
}

void configParser::parseScanner(rapidxml::xml_node<>* root){
    rapidxml::xml_node<>* scannerNode = root->first_node("scanner");
    if(scannerNode->first_attribute("enabled")->value() == std::string("true")){
        _scanner.startFreq = (uint64_t)std::stod(scannerNode->first_node("start_frequency")->value());
        _scanner.bandwidth = std::stod(scannerNode->first_node("bandwidth")->value());
        _scanner.numBands  = std::stod(scannerNode->first_node("num_bands")->value());
        _scanner.dwellTime = std::stof(scannerNode->first_node("dwell_time")->value());
        _scanner.enabled   = true;
    }else{
        _scanner.enabled = false;
    }
}

std::string configParser::getDatabaseName(){
    return _dbName;
}

void configParser::getDetectorParams(sdr::detectorParams& detector){
    detector.fftSize      = _detector.fftSize;
    detector.pfa          = _detector.pfa;
    detector.numAvgBins   = _detector.numAvgBins;
    detector.numGuardBins = _detector.numGuardBins;
    detector.windowType   = _detector.windowType;
}

void configParser::getRadioParams(sdr::deviceParams& frontEnd){
    frontEnd.baseBandFiltBw = _frontEnd.baseBandFiltBw;
    frontEnd.centerFreq     = _frontEnd.centerFreq;
    frontEnd.rxLnaGain      = _frontEnd.rxLnaGain;
    frontEnd.rxVgaGain      = _frontEnd.rxVgaGain;
    frontEnd.txVgaGain      = _frontEnd.txVgaGain;
    frontEnd.sampRate       = _frontEnd.sampRate;
}

void configParser::getScannerParams(sdr::scannerParams& scanner){
    if(_scanner.enabled){
        scanner.bandwidth = _scanner.bandwidth;
        scanner.dwellTime = _scanner.dwellTime;
        scanner.enabled   = _scanner.enabled;
        scanner.numBands  = _scanner.numBands;
        scanner.startFreq = _scanner.startFreq;
    }else{
        scanner.enabled = false;
    }
}
