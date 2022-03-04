# Beachmark

## 测试背景

+ 测试命令
```bash
ab -n 1000000 -c 100 -k http://127.0.0.1:18180/version
```


+ 测试机器（本机）
```
CPU: Intel(R) Core(TM) i7-4712MQ CPU @ 2.30GHz   2.29 GHz
RAM: 16.0 GB (15.7 GB 可用)
```

+ 测试条件

因为目前aproton暂时没有支持多进程，所以在单进程场景下进行了测试。

```
PHP版本： 
PHP 7.4.3 (cli) (built: Oct 25 2021 18:20:54) ( NTS )

Swoole版本： 
Version => 4.8.6
Built => Feb 13 2022 11:43:40
```

## APROTON
测试结果
```
Server Software:        proton/1.0
Server Hostname:        127.0.0.1
Server Port:            18180

Document Path:          /version
Document Length:        24 bytes

Concurrency Level:      100
Time taken for tests:   21.828 seconds
Complete requests:      1000000
Failed requests:        0
Keep-Alive requests:    1000000
Total transferred:      132000000 bytes
HTML transferred:       24000000 bytes
Requests per second:    45813.00 [#/sec] (mean)
Time per request:       2.183 [ms] (mean)
Time per request:       0.022 [ms] (mean, across all concurrent requests)
Transfer rate:          5905.58 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       4
Processing:     0    2   0.5      2      12
Waiting:        0    2   0.5      2      12
Total:          0    2   0.5      2      12

Percentage of the requests served within a certain time (ms)
  50%      2
  66%      2
  75%      2
  80%      2
  90%      3
  95%      3
  98%      4
  99%      4
 100%     12 (longest request)
```


## Swoole
测试结果
```
Server Software:        swoole-http-server
Server Hostname:        127.0.0.1
Server Port:            18180

Document Path:          /version
Document Length:        24 bytes

Concurrency Level:      100
Time taken for tests:   40.425 seconds
Complete requests:      1000000
Failed requests:        0
Keep-Alive requests:    1000000
Total transferred:      177000000 bytes
HTML transferred:       24000000 bytes
Requests per second:    24737.29 [#/sec] (mean)
Time per request:       4.042 [ms] (mean)
Time per request:       0.040 [ms] (mean, across all concurrent requests)
Transfer rate:          4275.88 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       3
Processing:     0    4   0.8      4      15
Waiting:        0    4   0.8      4      15
Total:          0    4   0.8      4      18

Percentage of the requests served within a certain time (ms)
  50%      4
  66%      5
  75%      5
  80%      5
  90%      5
  95%      5
  98%      5
  99%      5
 100%     18 (longest request)
```