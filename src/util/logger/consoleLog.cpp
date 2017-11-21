#include "consoleLog.h"

#include <stdarg.h>
#include <iostream>


void console::send(std::string color, std::string strRep, std::string origin, int lineNumber, std::string msg){
  //form message
  std::cout << color
            << "[" << strRep
            << " | " << origin
            << " | " << lineNumber << "]\t"
            << msg
            << consolereset << "\n";

}

void console::info(std::string origin, int lineNumber, std::string msg, ...){
    char     *args_msg;
    va_list   args;
    va_start(args, msg);
    int tmp = vasprintf(&args_msg, msg.c_str(), args);
    if(tmp)
        console::send(consoleINFO,strRepINFO,origin,lineNumber,args_msg);
    va_end(args);
    free(args_msg);
}

void console::debug(std::string origin, int lineNumber, std::string msg, ...){
    char     *args_msg;
    va_list   args;
    va_start(args, msg);
    int tmp = vasprintf(&args_msg, msg.c_str(), args);
    if(tmp)
        console::send(consoleDEBUG,strRepDEBUG,origin,lineNumber,args_msg);
    va_end(args);
    free(args_msg);
}

void console::warn(std::string origin, int lineNumber, std::string msg, ...){
    char     *args_msg;
    va_list   args;
    va_start(args, msg);
    int tmp = vasprintf(&args_msg, msg.c_str(), args);
    if(tmp)
        console::send(consoleWARNING,strRepWARNING,origin,lineNumber,args_msg);
    va_end(args);
    free(args_msg);
}

void console::error(std::string origin, int lineNumber, std::string msg, ...){
    char     *args_msg;
    va_list   args;
    va_start(args, msg);
    int tmp = vasprintf(&args_msg, msg.c_str(), args);
    if(tmp)
        console::send(consoleERROR,strRepERROR,origin,lineNumber,args_msg);
    va_end(args);
    free(args_msg);
}
