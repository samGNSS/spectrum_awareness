#include "udpBase.h"

#include <unistd.h>

udpSender* udpSender::inst = 0;

udpSender* udpSender::getInstance(){
    if(inst == 0){
        inst = new udpSender();
    }
    return inst;
};

udpSender::udpSender(){
    log = new console();
}

udpSender::~udpSender(){
    if(inst){
        delete inst;
        delete log;
    }
}

int udpSender::init(int port){
    //set up threads
    enabled = true;
    queueMonitor = std::thread(std::bind(&udpSender::watchQueue, this));
    
    //get socket and return
    if ((this->sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        log->error(__FILENAME__,__LINE__,"Failed to create socket");
        sleep(10);
    }
    
    memset((char *)&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;
    sAddr.sin_addr.s_addr = htonl(0x7F000001);
    sAddr.sin_port = htons(port);
    
    return this->sock;
}


void udpSender::watchQueue(){
    while(enabled){
        if(detQueue.size() > 2){
            detMtx.lock();
            while(detQueue.size() > 0){
                detVec.push_back(detQueue.front());
                detQueue.pop();
            }
            detMtx.unlock();
            sendDets();
        }
        if(iQQueue.size() > 100){
            //sendDets();
        }
        usleep(1000);
    }
}


void udpSender::pubIQ(radar::cfloatIQ iqData){
    iqMtx.lock();
    iQQueue.emplace(iqData);
    iqMtx.unlock();
}

void udpSender::pubDets(std::vector<radar::cfarDet> dets){
    detMtx.lock();
    detQueue.emplace(dets);
    //log->info(__FILENAME__,__LINE__,"New DET");
    detMtx.unlock();
}


int udpSender::sendData(char* buf, int size){
    //log->info(__FILENAME__,__LINE__,"Sending data");
    int ret = sendto(this->sock, buf, size, 0, (struct sockaddr*)&sAddr,sizeof(sAddr));
    return ret;
}

void udpSender::sendDets(){
    int numBytes;
    for(std::vector<radar::cfarDet> localDet : detVec){
        for(radar::cfarDet det : localDet){
            numBytes = sendData(reinterpret_cast<char*>(&det),sizeof(det));
            if(numBytes < 0){
                log->error(__FILENAME__,__LINE__,"Error sending detections");
            }
        }
    }
    detVec.clear();
}


