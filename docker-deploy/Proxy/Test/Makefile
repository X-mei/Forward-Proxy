all: proxy
testThreadPool:ThreadPoolTest.cpp
	g++ -o testThreadPool ThreadPoolTest.cpp ../Code/ThreadPool.h -std=c++1z -lpthread -ggdb3
proxy:Main.cpp
	g++ -o proxy Main.cpp ../Code/*.cpp -std=c++1z -O2 -lpthread -ggdb3
testParser:ParserTest.cpp
	g++ -o testParser ParserTest.cpp ../Code/*.cpp -std=c++1z -O2 -lpthread -ggdb3
.PHONY:
	clean
clean:
	rm -rf *~ testThreadPool proxy testParser *.o
