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

