FROM gcc:latest

# These commands copy your files into the specified directory in the image
# and set that as the working location
COPY . /src/
WORKDIR /src/

# This command compiles your app using GCC, adjust for your source code
RUN apt-get update && apt-get install -y make && apt-get install -y g++
# RUN g++ -o myapp main.cpp

# This command runs your application, comment out this line to compile only
# CMD ["./myapp"]
ENTRYPOINT ["/bin/sh","./run.sh"]

LABEL Name=httpcachingproxy Version=0.0.1
