//UDP class to send data to other stuff...like a python gui

#ifndef __udp_base__
#define __udp_base__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <cstdio>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../util/radarDataTypes.h"
#include "../util/logger/consoleLog.h"

class udpSender{
public:
  //member functions  
  static udpSender* getInstance();
  ~udpSender();
  int init(int port);
  void watchQueue();
  void pubIQ(radar::cfloatIQ iqData);
  void pubDets(std::vector<radar::cfarDet> dets);
  void sendDets();
  void sendIq();
  
private:
  udpSender();
  static udpSender* inst;
  
  int sendData(char* buf,int size);
  
  //condition variable
  //std::condition_variable sendDets;
  //std::condition_variable sendIq;
  
  //threads
  std::thread queueMonitor;
  bool enabled;

  //mutexes to protect the queues
  std::mutex detMtx;
  std::mutex iqMtx;
  
  //queues
  std::queue<std::vector<radar::cfarDet>> detQueue;
  std::vector<std::vector<radar::cfarDet>> detVec;
  std::queue<radar::cfloatIQ> iQQueue;
  
  //socket
  int sock;
  struct sockaddr_in sAddr;
  struct sockaddr_in retAddr;

  
  //pretty prints
  console* log;
};


// class udpReceiver{
// public:
//   
// private:
//   
//};

#endif
