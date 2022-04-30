all: proxy
proxy:Proxy.cpp
	g++ -o proxy Proxy.cpp ../Code/*.cpp -std=c++1z -O2 -lpthread -ggdb3

.PHONY:
	clean
clean:
	rm -rf *~ proxy *.o
