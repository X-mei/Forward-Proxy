all: proxy
proxy:Main.cpp
	g++ -o proxy Main.cpp ../Code/*.cpp -std=c++1z -O2 -lpthread -ggdb3
testParser:ParserTest.cpp
	g++ -o testParser ParserTest.cpp ../Code/*.cpp -std=c++1z -O2 -lpthread -ggdb3
testThreadPool:ThreadPoolTest.cpp
	g++ -o testThreadPool ThreadPoolTest.cpp ../Code/ThreadPool.h -std=c++1z -lpthread -ggdb3
testLog:LogTest.cpp
	g++ -o testLog LogTest.cpp ../Code/Log.cpp -std=c++1z -lpthread -ggdb3

.PHONY:
	clean
clean:
	rm -rf *~ testThreadPool proxy testParser testLog *.o
