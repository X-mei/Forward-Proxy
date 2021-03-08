## 1. Restricted Request Method
When parsing client request, we do not accept any method other than GET POST CONNECT, an exception is thrown immediately after we see such a method, since there are a lot more request method, this might make the process of accessing some website very slow or impossible

## 2. `detach()` instead of `join()`
We initiate thread in a method in `proxy` that runs forever(`while(1)`). A new thread is generated every time a request is accepted. We first used `join()` to sycronize between thread. But this result in some complex website unable to load all asset. This is because the main thread will block until the invoked thread finishes. However, big website is often loaded by sending different request sequentially/simultaneously, so we have to `detach()` the thread from the `thread` object so the main loop can continue to handle new request.

## 3. Exception Gaurantee
Our program provides a strong gaurantee. In any of the phase related to socket connection, a exception is thrown when they return -1. If the data received contains invalid field or malformed in any way that can not be parsed correctly. All these exception is cathed on the level of `handle()`. File descriptor for sever and client will be closed before ending the thread.

## 4. RAII design
In order to exploit edge of RAII design, we tried to minimize the case we call new. The only time we allocate a heap memory is when we are creating a `Request` object. And we have make sure that it is deleted after the processing is done, no matter if exception occured.

## 5. Cache
