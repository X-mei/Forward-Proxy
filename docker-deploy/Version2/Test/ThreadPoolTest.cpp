#include "../Code/ThreadPool.h"
#include <iostream>
#include <time.h>
#include <thread>
#include <vector>

using namespace std;

void printAnswer(vector<int> data, int i){
    data[i] += i;
}

int main(){
    clock_t start, end;
    vector<int> data(100000,0);
    ThreadPool pool(200);
    start = clock();
    for (int i=0; i<100000; ++i){
        pool.enqueue(bind(&printAnswer, data, i));
    }
    end = clock();
    cout << "Thread pool took: " << (double)(end-start)/CLOCKS_PER_SEC << endl;
    start = clock();
    for (int i=0; i<100000; ++i){
        thread trd(printAnswer, data, i);
        trd.detach();
    }
    end = clock();
    cout << "Normal Thread took: " << (double)(end-start)/CLOCKS_PER_SEC << endl;
    return 0;
}