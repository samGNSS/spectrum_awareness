#ifndef __CONSOLE_LOG__
#define __CONSOLE_LOG__

#include <string>
#include <sstream>
#include <vector>

// #define MAX_MSG_SIZE 1024

class console{
public:
  console();
  ~console();
  void info(std::string origin,int lineNumber,std::string msg,...);
  void debug(std::string origin,int lineNumber,std::string msg,...);
  void warn(std::string origin,int lineNumber,std::string msg,...);;
  void error(std::string origin,int lineNumber,std::string msg,...);
  
private:
  
  enum lvl {
    INFO = 0,
    DEBUG,
    WARNING,
    ERROR,
    reset
  };
  
  std::vector<std::string> color;
  std::vector<std::string> strRep;
    
  void send(lvl, std::string origin, int lineNumber, std::string msg);
};

#endif