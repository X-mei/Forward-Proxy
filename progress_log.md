## 1. Restricted Request Method
When parsing client request, we do not accept any method other than GET POST CONNECT, an exception is thrown immediately after we see such a method, since there are a lot more request method, this might make the process of accessing some website very slow or impossible

## 2. `detach()` instead of `join()`
We initiate thread in a method in `proxy` that runs forever(`while(1)`). A new thread is generated every time a request is accepted. We first used `join()` to sycronize between thread. But this result in some complex website unable to load all asset. This is because the main thread will block until the invoked thread finishes. However, big website is often loaded by sending different request sequentially/simultaneously, so we have to `detach()` the thread from the `thread` object so the main loop can continue to handle new request.

## 3. Exception Gaurantee
Our program provides a strong gaurantee. In any of the phase related to socket connection, a exception is thrown when they return -1. If the data received contains invalid field or malformed in any way that can not be parsed correctly. All these exception is cathed on the level of `handle()`. File descriptor for sever and client will be closed before ending the thread.

## 4. RAII design
In order to exploit edge of RAII design, we tried to minimize the case we call new. The only time we allocate a heap memory is when we are creating a `Request` object. And we have make sure that it is deleted after the processing is done, no matter if exception occured.

## 5. Issues encountered
### 02/27
- We use a long string to store the received data from both client and server. However, one potential danger exists in string is that string will automatically end in the ‘\0’ character, and the data transmitted may contain ‘\0’ in the middle of its content and it could lead to lost of message. A possible solution is to use a char array instead of string.
- The parsing of http url in request header in our proxy follows the basic pattern. However, the http header may use other special formats and it could lead to the failure of parsing http request.

### 03/01
- The parsing of header field only follows the common pattern. For example, we assume that the time value of Date is in GMT format, while the time could be not in GMT format and it could lead to failure of our cache validation.
- Our proxy analyze the cache control only according to the “Cache Control” header. However in our test it is found that a few websites use other header field instead of “Cache Control”.
- For now we only test the functionality of the cache system using self-made requests and responses (in string). It will not guarantee the success in real connection. So we will implement tests in real connections soon.  

### 03/03
- The http header fields in a few websites may be different from the normal ones. For example, some websites use all lower case characters.
- Our cache size is set to 100, but it could be easily filled if we have many requests.
- We simply evict the least recently used response if the cache is full. However, this response may still be fresh and another strategy would be to evict the out-dated responses first.

### 03/04
- For now, if our proxy encounters a situation that it cannot handle, it will simply print the error message and exit. We will add some error catching conditions to avoid the use of exit() and handle those situations.
- Our client could send bad request like a host that does not exist. We should add more error control system to handle these bad request.
- The efficiency of our proxy is low, we should implement multi-thread operations.

### 03/06
- Only one thread should be able to get access to, including reading and writing to our cache data, which is stored in a map. So we add mutex in the critical regions.
- If the number of request is too large, our proxy may crush.
- We may receive harmful request from client, and our proxy is fragile to attacks.

### 04/13/2022
- CONNECT method requires a lot of thread resources, a website with lots of resources typically needs multiple CONNECT requests for it to load properly.
- Non-blocking IO would interfare with select, maybe because non-blocking fd returns immediately, which breaks the polling process while blocking IO wait until disconnection or timeout.

### 04/17/2022
- The reason why epoll didn't work on write is that the associated file descriptor is already close. Hence made update to the life cycle control of file descriptors.

### 04/19/2022
- Did basic load testing on the server, could achieve roughly 1000 QPS. Running on a ubuntu 20 server with 2 core and 2G of memory.

### 04/21/2022
- Added testing for log & fixed issue of log not flushed to file.
- Depending on the file stream mode (full buff, line buff, no buff, default full buff), stream is flushed to disk at different time in the execution.
- `fclose()` and `fflush()` are two ways to make sure stream is flushed to the file. The bug has to do with two thread not syncronized, that is, `fflush()` is called in one thread before the `fputs()` in the other thread is executed.