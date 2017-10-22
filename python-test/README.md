这是nginx cosocket 和 python tornado 的一次性能对比， nginx cosocket 基本是tornado 的近20倍， 下列程序都是在单核cpu模式下运行, 详细性能对比看下图


[ym@localhost sbin]$ ab -n 100000 -c 100 -k http://127.0.0.1:8000/test
This is ApacheBench, Version 2.3 <$Revision: 1807734 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        nginx/1.13.6
Server Hostname:        127.0.0.1
Server Port:            8000

Document Path:          /test
Document Length:        98 bytes

Concurrency Level:      100
Time taken for tests:   7.279 seconds
Complete requests:      100000
Failed requests:        0
Keep-Alive requests:    99049
Total transferred:      24595245 bytes
HTML transferred:       9800000 bytes
Requests per second:    13738.85 [#/sec] (mean)
Time per request:       7.279 [ms] (mean)
Time per request:       0.073 [ms] (mean, across all concurrent requests)
Transfer rate:          3299.91 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0       8
Processing:     1    7   3.1      7     145
Waiting:        1    7   3.1      7     145
Total:          1    7   3.1      7     147

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      8
  75%      8
  80%      8
  90%      8
  95%      8
  98%      9
  99%     10
 100%    147 (longest request)
[ym@localhost sbin]$ ab -n 100000 -c 100 -k http://127.0.0.1:8002/test
This is ApacheBench, Version 2.3 <$Revision: 1807734 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
apr_socket_recv: Connection refused (111)
[ym@localhost sbin]$ ab -n 100000 -c 100 -k http://127.0.0.1:8002/test
This is ApacheBench, Version 2.3 <$Revision: 1807734 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        TornadoServer/4.5.2
Server Hostname:        127.0.0.1
Server Port:            8002

Document Path:          /test
Document Length:        8 bytes

Concurrency Level:      100
Time taken for tests:   128.403 seconds
Complete requests:      100000
Failed requests:        0
Keep-Alive requests:    100000
Total transferred:      22600000 bytes
HTML transferred:       800000 bytes
Requests per second:    778.80 [#/sec] (mean)
Time per request:       128.403 [ms] (mean)
Time per request:       1.284 [ms] (mean, across all concurrent requests)
Transfer rate:          171.88 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0       9
Processing:    66  128   6.6    128     316
Waiting:       66  128   6.6    128     316
Total:         74  128   6.6    128     316

Percentage of the requests served within a certain time (ms)
  50%    128
  66%    128
  75%    129
  80%    129
  90%    130
  95%    131
  98%    134
  99%    140
 100%    316 (longest request)

