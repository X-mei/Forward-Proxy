FROM gcc:latest

COPY ./Code ./Code/

COPY ./Run ./Run/

WORKDIR /Run/

RUN apt-get update && apt-get install -y make && apt-get install -y g++

ENTRYPOINT ["/bin/sh","./run.sh"]

