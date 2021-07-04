#ifndef LOG_H
#define LOG_H
#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir

class Log{
private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path;
    const char* suffix;

    bool is_open;
    int level;
    int line_count;
    int this_day;
    int is_async;

    FILE* fp;
    // RAII here
    std::unique_ptr<BlockDeque<std::string>> my_deque;
    std::unique_ptr<std::thread> write_thread;
    std::mutex mtx;

    // Buffer buff;
    // int MAX_LINES; // ????
    
    // singleton, need cons/dest as private/protected
    Log();
    virtual ~Log();
    void AppendLogLevelTitle(int level);
    void AsyncWrite();

public:
    void Init(int level, const char* path = "./log", const char* suffix = ".log", int maxQueueCapacity = 1024);
    
    static Log* GetInstance();//Instance()
    static void FlushLogThread();

    void Write(int level, const char* format, ...);
    void Flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return is_open; }

};

// the ... and ##__VA_ARGS__here is used to define a variadic macros to conform to different type and number of parameters
#define LOG_BASE(level, format, ...) \
    do { \
        Log* log = Log::GetInstance(); \
        if (log->IsOpen() && log->getLevel() <= level){ \
            log->write(level, format, ##__VA_ARGS__); \
            log->flush(); \
        } \
    } while(0);

// define some type of logs
// the do{...}while(0) here is a trick used widely in defination
// It is 1. the only construct in C that you can use to #define a multistatement operation, put a semicolon after, and still use within an if statement.
// 2. avoid goto which makes code ugly and complicated in #define
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif