编译:
nginx中甬道c++代码(降低开发代价),所以需要修改cc/conf文件
./configure --add-module=src/cosocket --add-module=src/test --with-ld-opt="-L/usr/local/lib/ -lhiredis"
需要hireids库，如果你用其他的库， 这里宝hiredis改称对应的库



在nginx 中调用阻塞库，阻塞库中包含sleep, usleep, 阻塞的socket, 但不包括文件IO(文件IO只能用多线程这种方式优化，参照nginx的实现方式)，nginx仍然能高并发处理。

技术：
nginx-cosocket = nginx + coroutine + hook
1:通过部分修改libc库函数的行为， 让nginx 调用这些阻塞库时仍然能保持高并发
2:使用coroutine, 让上层保持“阻塞or sleep” 的行为， 当这些事件发生时， nginx 会切换到coroutine 中执行， 当上层运行的条件没有达成时， 自动切换到nginx运行， 减少nginx开发的复杂度
3:暂时之测试了redis库(下图是在i3(2.9GHZ测试的，nginx单核运行))
Server Software:        nginx/1.13.6
Server Hostname:        127.0.0.1
Server Port:            8000

Document Path:          /test
Document Length:        10 bytes

Concurrency Level:      100
Time taken for tests:   21.283 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      15300000 bytes
HTML transferred:       1000000 bytes
Requests per second:    4698.68 [#/sec] (mean)
Time per request:       21.283 [ms] (mean)
Time per request:       0.213 [ms] (mean, across all concurrent requests)
Transfer rate:          702.05 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   2.0      0      18
Processing:     3   20   5.6     19      80
Waiting:        3   20   5.4     18      71
Total:          8   21   5.3     20      80

Percentage of the requests served within a certain time (ms)
  50%     20
  66%     22
  75%     24
  80%     25
  90%     27
  95%     30
  98%     35
  99%     40
 100%     80 (longest request)

现在 ngx_http_test_module模块使用短连接连接redis, 以后使用长连接这个速率可能达到1W左右， 基本满足要求
4：可以通过修改redis_test()方法，去测试其他程序

cosocket 是ngx_cosocket核心代码， 里面包括中写的库代码
test 是redis_test测试代码
5: 现在每一个协程的栈空间都是64K， 可以更具需要修改


6：最新支持后段服务长连接
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
Document Length:        10 bytes

Concurrency Level:      100
Time taken for tests:   8.621 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      15300000 bytes
HTML transferred:       1000000 bytes
Requests per second:    11599.05 [#/sec] (mean)
Time per request:       8.621 [ms] (mean)
Time per request:       0.086 [ms] (mean, across all concurrent requests)
Transfer rate:          1733.06 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    2   1.5      3       7
Processing:     3    6   1.6      6      36
Waiting:        2    5   2.0      4      35
Total:          5    9   1.5      9      39

Percentage of the requests served within a certain time (ms)
  50%      9
  66%      9
  75%      9
  80%      9
  90%     10
  95%     11
  98%     11
  99%     12
 100%     39 (longest request)

是第五的两倍，还是廷强悍的

7: 前端也是用长连接测试
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
Document Length:        10 bytes

Concurrency Level:      100
Time taken for tests:   7.387 seconds
Complete requests:      100000
Failed requests:        0
Keep-Alive requests:    99048
Total transferred:      15795240 bytes
HTML transferred:       1000000 bytes
Requests per second:    13537.92 [#/sec] (mean)
Time per request:       7.387 [ms] (mean)
Time per request:       0.074 [ms] (mean, across all concurrent requests)
Transfer rate:          2088.23 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0       8
Processing:     1    7   3.2      7     156
Waiting:        1    7   3.1      7     155
Total:          1    7   3.2      7     157

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      8
  75%      8
  80%      8
  90%      8
  95%      8
  98%      9
  99%     10
 100%    157 (longest request)


这个性能也是不错的
