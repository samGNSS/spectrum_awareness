#ifndef __DB_LOGGER__
#define __DB_LOGGER__

#include <string>
#include <sqlite_modern_cpp.h>
#include <vector>

#include "../radarDataTypes.h"

class databaseLogger{
public:
    static databaseLogger* getInst(std::string dbName);
    static databaseLogger* getInst();
    ~databaseLogger();
    
    void logDets(std::vector<radar::cfarDet> dets);
    //void logChannels(std::vector<radar::cfarDet> dets);
private:    
    databaseLogger(std::string dbName);
    static databaseLogger* inst;
    
    void detLoggerWorker(); 
        
    //db pointer
    sqlite::database* db;
        
    //reference counter
    static size_t refCount;
};

#endif
