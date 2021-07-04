#include "log.h"

Log::Log() {
    line_count = 0;
    is_async = false;
    write_thread = nullptr;
    my_deque = nullptr;
    this_day = 0;
    fp = nullptr;
}

Log::~Log() {
    if(write_thread && write_thread->joinable()) {
        while(!my_deque->empty()) {
            my_deque->flush();
        };
        my_deque->Close();
        write_thread->join();
    }
    if(fp_) {
        lock_guard<mutex> locker(mtx);
        Flush();
        fclose(fp);
    }
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
    this->is_open = true;
    this->level = level;
    if(maxQueueCapacity > 0) {
        this->is_async = true;
        if(!my_deque) {
            unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            this->my_deque = move(newDeque);
            
            std::unique_ptr<std::thread> NewThread(new thread(FlushLogThread));
            this->write_thread = move(NewThread);
        }
    } 
    else {
        this->is_async = false;
    }

    line_count = 0;

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    this->path = path;
    this->suffix = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            this->path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, this->suffix);
    this_day = t.tm_mday;
    {
        lock_guard<mutex> locker(mtx);
        buff.RetrieveAll();
        if(fp) { 
            flush();
            fclose(fp); 
        }

        fp = fopen(fileName, "a");
        if(fp == nullptr) {
            mkdir(this->path, 0777);
            fp = fopen(fileName, "a");
        } 
        assert(fp != nullptr);
    }
}

void Log::Write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    // if reached next day or the line count is maxed out/new page, need to make a new file
    if (this_day != t.tm_mday || (line_count && (line_count  %  MAX_LINES == 0)))
    {
        unique_lock<mutex> locker(mtx);
        locker.unlock();
        
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        // create tail
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (this_day != t.tm_mday) { // if a new day
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", this->path, tail, this->suffix);
            this_day = t.tm_mday;
            line_count = 0;
        }
        else { // if not a new day but the line count has reached maximum line count, need extra page for this day
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", this->path, tail, (line_count / MAX_LINES), this->suffix);
        }
        locker.lock();
        this->Flush();
        fclose(fp);
        fp = fopen(newFile, "a");
        assert(fp != nullptr);
    }

    // write content into the file(old or new)
    {
        unique_lock<mutex> locker(mtx);
        ++line_count;
        int n = snprintf(buff.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
                    
        buff.HasWritten(n);
        this->AppendLogLevelTitle(level);

        // va_start & va_end & va_list are used with the varadic variables ...
        // vsnprintf funtion similar to snprintf, but it takes in va_list as the variable argument list
        va_start(vaList, format);
        int m = vsnprintf(buff.BeginWrite(), buff.WritableBytes(), format, vaList);
        va_end(vaList);

        buff.HasWritten(m);
        buff.Append("\n\0", 2);

        // depending on the mode of writing, use asyncronous mode/direct write
        if(is_async && my_deque && !my_deque->full()) {
            my_deque->push_back(buff.RetrieveAllToStr());
        } else {
            fputs(buff.Peek(), fp);
        }
        buff.RetrieveAll();
    }
}

void Log::AppendLogLevelTitle(int level) {
    switch(level) {
        case 0:
            buff.Append("[debug]: ", 9);
            break;
        case 1:
            buff.Append("[info]: ", 9);
            break;
        case 2:
            buff.Append("[warning]: ", 9);
            break;
        case 3:
            buff.Append("[error]: ", 9);
            break;
        default:
            buff.Append("[info]: ", 9);
            break;
    }
}

void Log::Flush() {
    if (is_async){
        my_deque->flush();
    }
    fflush(fp);
}

void Log::AsyncWrite() {
    string str = "";
    while (my_deque->pop(str)) {
        lock_guard<mutex> locker(mtx);
        fputs(str.c_str(), fp);
    }
}

// using idea called local static to realize singleton
Log* Log::GetInstance() {
    static Log instance;
    return &instance;
}

void Log::FlushLogThread() {
    Log::GetInstance()->AsyncWrite();
}
