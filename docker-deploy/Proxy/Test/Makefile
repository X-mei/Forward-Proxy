all: test1 proxy
test1:ThreadPoolTest.cpp
	g++ -o test1 ThreadPoolTest.cpp ../Code/ThreadPool.h -std=c++1z -lpthread -ggdb3
proxy:Main.cpp
	g++ -o proxy Main.cpp ../Code/*.cpp -std=c++1z -O2 -lpthread -ggdb3

.PHONY:
	clean
clean:
	rm -rf *~ test1 proxy *.o *.txt
