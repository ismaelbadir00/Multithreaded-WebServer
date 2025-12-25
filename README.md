

\# Multi-threaded Web Server in C (Thread Pool + Bounded Queue)



A high-performance \*\*multi-threaded HTTP server\*\* written in \*\*C\*\* using \*\*pthreads\*\*.

It uses a \*\*fixed-size thread pool\*\* and a \*\*bounded request queue\*\* with multiple \*\*overload handling policies\*\*.



This project demonstrates real systems skills recruiters care about:

\*\*concurrency\*\*, \*\*synchronization\*\*, \*\*producer/consumer design\*\*, \*\*queueing policies\*\*, and \*\*performance instrumentation\*\*.



---



\## Highlights



\- \*\*Thread pool\*\*: fixed number of worker threads handling requests concurrently

\- \*\*Bounded queue\*\*: backpressure + configurable overload behavior

\- \*\*Static + dynamic (CGI) support\*\*

\- \*\*Request instrumentation\*\* via HTTP response headers:

&nbsp; - queueing / dispatch time

&nbsp; - per-thread counters (handled / static / dynamic totals)



---



\## Overload Handling Policies



When the queue is full, the server supports:



\- `block` : block the acceptor until space is available  

\- `dt`    : drop-tail (drop the new request)

\- `dh`    : drop-head (drop the oldest queued request, enqueue the new one)

\- `bf`    : block-flush (wait until queue empties, then drop the incoming request)

\- `random` (bonus): drop ~50% of queued requests randomly, then enqueue the new one



---



\## Special Behavior: `.skip`



If a dequeued request ends with `.skip`, the worker:



1\. Strips the `.skip` suffix  

2\. Dequeues the \*\*most recent\*\* request currently waiting in the queue  

3\. Serves the `.skip` request first, then serves that dequeued request  



This requires careful queue manipulation under concurrency and correct synchronization.



---



\## Build



```bash

make

````



---



\## Run



```bash

./server <port> <num\_threads> <queue\_size> <schedalg>

```



Example:



```bash

./server 5003 8 16 dt

```



---



\## Test



Using the provided client:



```bash

./client localhost 5003 home.html

```



Or directly from a browser:



```

http://localhost:5003/home.html

```



You can also inspect headers using curl:



```bash

curl -i http://localhost:5003/home.html | head -n 30

```



---



\## Response Statistics (Observability)



Each HTTP response includes runtime statistics via headers.



\### Per-request



\* `Stat-Req-Arrival` – timestamp when request arrived to the master thread

\* `Stat-Req-Dispatch` – time spent waiting in the queue before a worker handled it



\### Per-thread



\* `Stat-Thread-Id`

\* `Stat-Thread-Count` – total handled requests

\* `Stat-Thread-Static` – static requests served

\* `Stat-Thread-Dynamic` – dynamic (CGI) requests served



These headers make it easy to verify correctness and analyze performance under load.



---



\## Design Notes



\* \*\*Producer–consumer model\*\*:



&nbsp; \* Master thread accepts connections and enqueues requests

&nbsp; \* Worker threads dequeue and process requests

\* \*\*Synchronization\*\*:



&nbsp; \* `pthread\_mutex\_t` protects shared queue and counters

&nbsp; \* `pthread\_cond\_t` used for blocking (no busy-waiting)

\* \*\*Scheduling decisions\*\* are made explicitly at enqueue time

\* Designed to be \*\*deadlock-free\*\*, \*\*race-free\*\*, and \*\*fair\*\* under load



---



\## Tech Stack



\* C

\* POSIX sockets

\* pthreads

\* CGI (dynamic content)



---



