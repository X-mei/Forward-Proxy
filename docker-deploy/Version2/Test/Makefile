all: test1 test2
test1:ThreadPoolTest.cpp
	g++ -o test1 ThreadPoolTest.cpp ../Code/ThreadPool.h -std=c++1z -lpthread -ggdb3
test2:EpollTest.cpp
	g++ -o test2 EpollTest.cpp ../Code/*.cpp -std=c++1z -O2 -lpthread -ggdb3

.PHONY:
	clean
clean:
	rm -rf *~ test1 test2 *.o *.txt
