#include "memBuffer.h"

memBuff* memBuff::inst = 0;

memBuff::memBuff(){
    enabled = true;
    ageOut = std::thread(std::bind(&memBuff::ageOutTimer, this));
    log = new console();
    age = std::chrono::seconds(10);
    //db = databaseLogger::getInst();
}

memBuff::~memBuff(){
    log->info(__FILENAME__,__LINE__,"Stopping mem buffer");
    enabled = false;
    ageOut.join();
    delete log;
    //delete db;
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
    uint64_t keyVal = dets.at(0).timeOn;
    detIt = detMap.find(keyVal);
    detMap.insert(detIt,std::pair<uint64_t,std::vector<radar::cfarDet>>(keyVal,dets));
    detMtx.unlock();
}

std::vector<std::vector<radar::cfarDet>> memBuff::getDetsDataInRange(uint64_t startTime, uint64_t endTime){
    detMtx.lock();
    std::map<uint64_t,std::vector<radar::cfarDet>>::iterator lowIt  = detMap.lower_bound(startTime);
    std::map<uint64_t,std::vector<radar::cfarDet>>::iterator highIt = detMap.upper_bound(endTime);
    detMtx.unlock();
    
    std::vector<std::vector<radar::cfarDet>> ret;
    for(;lowIt!=highIt;++lowIt){
        ret.push_back(lowIt->second);
    }
    
    return ret;
}

void memBuff::ageOutDets(uint64_t startTime){
    log->debug(__FILENAME__,__LINE__,"Map size before: %d",detMap.size());
    detMtx.lock();
    std::map<uint64_t,std::vector<radar::cfarDet>>::iterator highIt = detMap.upper_bound(startTime);
    for(std::map<uint64_t,std::vector<radar::cfarDet>>::iterator it = detMap.begin();it!=highIt;++it){
        //db->logDets(it->second);
        detMap.erase(it);
    }
    detMtx.unlock();
    log->debug(__FILENAME__,__LINE__,"Map size after: %d",detMap.size());
}

void memBuff::ageOutTimer(){
    while(enabled){
        std::this_thread::sleep_for(age);
        if(enabled){
            ageOutDets(detMap.begin()->first);
        }
    }
}
