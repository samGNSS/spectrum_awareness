#include "memBuffer.h"

memBuff* memBuff::inst = 0;

memBuff::memBuff(){
    enabled = true;
    ageOut = std::thread(std::bind(&memBuff::ageOutTimer, this));
    log = new console();
    age = std::chrono::seconds(10);
}

memBuff::~memBuff(){
    if(inst!=0){
        enabled = false;
        ageOut.join();
        delete inst;
    }
}

memBuff* memBuff::getInst(){
    if(inst==0){
        inst = new memBuff();
    }
    return inst;
};

void memBuff::publishDets(std::vector<radar::cfarDet> dets){
    //insert into map
    detMtx.lock();
    for(radar::cfarDet det : dets){
        detIt = detMap.find(det.time);
        detMap.insert(detIt,std::pair<uint64_t,radar::cfarDet>(det.time,det));
    }
    detMtx.unlock();
}

std::vector<radar::cfarDet> memBuff::getDetsDataInRange(uint64_t startTime, uint64_t endTime){
    detMtx.lock();
    std::map<uint64_t,radar::cfarDet>::iterator lowIt = detMap.lower_bound(startTime);
    std::map<uint64_t,radar::cfarDet>::iterator highIt = detMap.upper_bound(endTime);
    detMtx.unlock();
    
    std::vector<radar::cfarDet> ret;
    for(;lowIt!=highIt;++lowIt){
        ret.push_back(lowIt->second);
    }
    
    return ret;
}

void memBuff::ageOutDets(uint64_t startTime){
    log->debug(__FILENAME__,__LINE__,"Map size before: %d",detMap.size());
    detMtx.lock();
    std::map<uint64_t,radar::cfarDet>::iterator highIt = detMap.upper_bound(startTime);
    for(std::map<uint64_t,radar::cfarDet>::iterator it = detMap.begin();it!=highIt;++it){
        detMap.erase(it);
    }
    detMtx.unlock();
    log->debug(__FILENAME__,__LINE__,"Map size after: %d",detMap.size());
}

void memBuff::ageOutTimer(){
    while(enabled){
        std::this_thread::sleep_for(age);
        ageOutDets(detMap.begin()->first + 10);
    }
}
