#include "databaseLogger.h"

databaseLogger* databaseLogger::inst = 0;
size_t databaseLogger::refCount = 0;

databaseLogger* databaseLogger::getInst(std::string dbName){
    ++refCount;
    if(inst == 0){
        inst = new databaseLogger(dbName);
    }
    return inst;
}

databaseLogger* databaseLogger::getInst(){
    if(inst != 0){
        ++refCount;
        return inst;
    }else{
        return nullptr;
    }

}

databaseLogger::~databaseLogger(){
    --refCount;
    if(refCount == 0){
        delete inst;
        delete db;
    }
}


databaseLogger::databaseLogger(std::string dbName){
    db = new sqlite::database(dbName);

    //create detection table
    this->db->operator<<("create table if not exists detections("\
                         "_id integer primary key autoincrement not null,"\
                         "timeOn long long, timeOnMicroSecs long long, center long long, bandwidth long, power float);");
}


void databaseLogger::logDets(std::vector<radar::target> dets){
    //get prepared statements
    auto insertIntoDets = this->db->operator<<("insert into detections (timeOn,timeOnMicroSecs,center,bandwidth,power) values (?,?,?,?,?);");
    for(radar::target det : dets){
        insertIntoDets << det.timeOn
                       << det.timeOnMicroSec
                       << det.freqHz.first
                       << det.freqHz.second
                       << det.power;
        insertIntoDets++;
    }
}
