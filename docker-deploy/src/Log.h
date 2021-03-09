#ifndef LOG_H
#define LOG_H
#include <fstream>
using namespace std;
#define LOG_PATH "/var/log/erss/proxy.log"

class Log {
private:
    ofstream logFile;
public:
    Log() {
        logFile.open(LOG_PATH, ios::app);
    }
    ~Log() {
        logFile.close();
    }
    void save(string msg) {
        cout << "======writing=========" << endl;
        logFile << msg << endl;
    }
};

#endif /* Log_h */
