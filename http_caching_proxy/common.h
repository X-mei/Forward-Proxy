#ifndef COMMON_H
#define COMMON_H

#include <list>
#include <ctime>
#include <mutex>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iostream>
#include <exception>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sstream>
#include <fstream>

#define LOG_PATH "/var/log/erss/proxy.log"

extern std::ofstream logFile;

#endif