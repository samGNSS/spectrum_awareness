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
    #define consoleINFO    "\033[96m"
    #define consoleDEBUG   "\033[92m"
    #define consoleWARNING "\033[95m"
    #define consoleERROR   "\033[91m"
    #define consolereset   "\033[0m"

    //simple strings to go with the colors
    #define strRepINFO    "INFO"
    #define strRepDEBUG   "DEBUG"
    #define strRepWARNING "WARNING"
    #define strRepERROR   "ERROR"



    //"base" functions
    void info(std::string origin,int lineNumber,std::string msg,...);
    void debug(std::string origin,int lineNumber,std::string msg,...);
    void warn(std::string origin,int lineNumber,std::string msg,...);;
    void error(std::string origin,int lineNumber,std::string msg,...);
    void send(std::string color, std::string strRep, std::string origin, int lineNumber, std::string msg);

    //macros around the base functions to emulate printf
    //TODO: make these work
    #define outINFO(x,...)(info(__FILENAME__,__LINE__),x,...)

};

#endif
