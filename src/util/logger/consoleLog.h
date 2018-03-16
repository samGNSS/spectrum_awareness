#ifndef __CONSOLE_LOG__
#define __CONSOLE_LOG__

#include <string>
#include <sstream>
#include <vector>

// #define MAX_MSG_SIZE 1024



namespace console{
    //file name macro
    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

    //console colors
    static const std::string consoleINFO    = "\033[96m";
    static const std::string consoleDEBUG   = "\033[92m";
    static const std::string consoleWARNING = "\033[95m";
    static const std::string consoleERROR   = "\033[91m";
    static const std::string consolereset   = "\033[0m";

    //simple strings to go with the colors
    static const std::string strRepINFO    = "INFO";
    static const std::string strRepDEBUG   = "DEBUG";
    static const std::string strRepWARNING = "WARNING";
    static const std::string strRepERROR   = "ERROR";

    //"base" functions
    void info(std::string origin,int lineNumber,std::string msg,...);
    void debug(std::string origin,int lineNumber,std::string msg,...);
    void warn(std::string origin,int lineNumber,std::string msg,...);;
    void error(std::string origin,int lineNumber,std::string msg,...);
    void send(std::string color, std::string strRep, std::string origin, int lineNumber, std::string msg);
};

#endif
