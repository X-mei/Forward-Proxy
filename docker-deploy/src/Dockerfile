FROM gcc

RUN mkdir /src
WORKDIR /src

RUN apt-get update && apt-get install -y make && apt-get install -y g++

ADD . /src

RUN chmod +rx main.cpp