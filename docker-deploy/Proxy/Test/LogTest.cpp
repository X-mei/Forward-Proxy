#include "../Code/Log.h"
#include <iostream>
#include <time.h>
#include <thread>
#include <vector>
#include <unistd.h>

using namespace std;

void write_to_log(int i){
    LOG_DEBUG("This is thread: %d", i);
    LOG_INFO("This is thread: %d", i);
    LOG_WARN("This is thread: %d", i);
    LOG_ERROR("This is thread: %d", i);
}

int main(){
    Log::Instance()->init(0, "./log", ".log", 1024, 1024);
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i){
        threads.push_back(std::thread(write_to_log, i));
    }
    sleep(10);
    // loop again to join the threads
    for (auto& t : threads){
        t.join();
    }
    
    return 0;
}