#include "log.h"

Log::Log() {

}

Log::~Log() {

}

int Log::GetLevel() {
    lock_guard<mutex> locker(mtx);
    return level;
}

void Log::SetLevel(int level) {
    lock_guard<mutex> locker(mtx);
    this->level = level;
}

void Log::Init(int level = 1, const char* path, const char* suffix, int maxQueueCapacity){
    
}
