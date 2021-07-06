all: test1 test2
test1:ThreadPoolTest.cpp
	g++ -o test1 ThreadPoolTest.cpp ../Version2/ThreadPool.h -std=c++1z -lpthread -ggdb3
test2:EpollTest.cpp
	g++ -o test2 EpollTest.cpp ../Version2/ProxyServer.cpp ../Version2/Epoll.cpp ../Version2/ThreadPool.h ../Version2/BlockQueue.h ../Version2/Log.cpp ../Version2/Buffer.cpp -std=c++1z -lpthread -ggdb3

.PHONY:
	clean
clean:
	rm -rf *~ test1 test2 *.o *.txt
