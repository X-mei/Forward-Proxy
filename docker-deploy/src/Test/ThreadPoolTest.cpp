#include "../Version2/ThreadPool.h"
#include <iostream>
#include <time.h>
#include <thread>

using namespace std;

void printAnswer(int answer){
    if (answer%10==0){
        cout << answer << endl;
    }
}

int main(){
    clock_t start, end;
    ThreadPool pool(30);
    start = clock();
    for (int i=0; i<100000; ++i){
        pool.enqueue(bind(&printAnswer, i));
    }
    end = clock();
    cout << "Thread pool took: " << (double)(end-start)/CLOCKS_PER_SEC << endl;
    start = clock();
    for (int i=0; i<100000; ++i){
        thread trd(printAnswer, i);
        trd.detach();
    }
    end = clock();
    cout << "Normal Thread took: " << (double)(end-start)/CLOCKS_PER_SEC << endl;
    return 0;
}