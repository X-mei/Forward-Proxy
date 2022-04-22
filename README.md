## Authors
Meihong Ge, mg476@duke.edu  
Ruiqi Sun, rs546@duke.edu
## To run the Proxy:
This is a developmental branch, it is runnable but would require compiling the source files manually. Please see the main branch, where there is a containerized version.

## Overall Proxy design:
We followed a good OO design principle when designing the code structure. The `proxy` is a class that assembles the main structure of the proxy. Connection is done by initating and calling class `socket`. The data sended between client and server is stored and parsed by two class called `Request` and `Response` that are child class of `HTTP`. Upon receiving data from either client or server, if we care certain field of the transmitted message(e.g, what is the method used? GET?POST?CONNECT?), a `Request` or `Response` instance is initated to parse the data. When a GET request is encountered, a field under `proxy` of class `Cache` is invoked to retirve cached response, update invalid field, and add map of unseen request.

## Cache Design:
Cache Size is set to 100. LRU policy is implemented to update caches. Once the capacity of cache is reached, clean the least recently used expired response. Cache control headers are analyzed in cache validation process and updation process.

## To Do:
- [x] Log overhaul needed.
    - [ ] Better utilization of buffer, performance mode & debugg mode
- [ ] Compile speed too slow.
- [ ] Containerization change on the new project layout.
- [ ] Better IO performance by polling both client side and server side.
- [ ] Stability, segmentation sometimes occurs.


## Referencce:
The socket connection is written following code provided in Beej's Guide: https://beej.us/guide/bgnet/html/  
It is written in C, so some minor modification is done to lessen the usage of C code.

The way we parse the HTTP request and response is mainly done by following the Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing: https://tools.ietf.org/html/rfc7230