#include "consoleLog.h"

#include <stdarg.h>
#include <iostream>

console::console(){
  color.resize(5);
  strRep.resize(4);
  color[INFO]    = "\033[96m";strRep[INFO]    = "INFO";
  color[DEBUG]   = "\033[92m";strRep[DEBUG]   = "DEBUG";
  color[WARNING] = "\033[95m";strRep[WARNING] = "WARNING";
  color[ERROR]   = "\033[91m";strRep[ERROR]   = "ERROR";
  color[reset]   = "\033[0m";

}
console::~console(){}

void console::send(lvl level, std::string origin, int lineNumber, std::string msg){
  //form message
  std::cout << color[level]
            << "[" << strRep[level]
            << " | " << origin
            << " | " << lineNumber << "]\t"
            << msg
            << color[reset] << "\n";

}

void console::info(std::string origin, int lineNumber, std::string msg, ...){
    char     *args_msg;
    va_list   args;
    va_start(args, msg);
    vasprintf(&args_msg, msg.c_str(), args);
    this->send(lvl::INFO,origin,lineNumber,args_msg);
    va_end(args);
    free(args_msg);
}

void console::debug(std::string origin, int lineNumber, std::string msg, ...){
    char     *args_msg;
    va_list   args;
    va_start(args, msg);
    vasprintf(&args_msg, msg.c_str(), args);
    this->send(lvl::DEBUG,origin,lineNumber,args_msg);
    va_end(args);
    free(args_msg);
}

void console::warn(std::string origin, int lineNumber, std::string msg, ...){
    char     *args_msg;
    va_list   args;
    va_start(args, msg);
    vasprintf(&args_msg, msg.c_str(), args);
    this->send(lvl::WARNING,origin,lineNumber,args_msg);
    va_end(args);
    free(args_msg);
}

void console::error(std::string origin, int lineNumber, std::string msg, ...){
    char     *args_msg;
    va_list   args;
    va_start(args, msg);
    vasprintf(&args_msg, msg.c_str(), args);
    this->send(lvl::ERROR,origin,lineNumber,args_msg);
    va_end(args);
    free(args_msg);
}
