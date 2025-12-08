# webserv

Comment Notes:

TEMP~ 	- I will change it back later
FAIL~ 	- Clean up is not done yet
DANGER~ - DONT TOUCH!!!

Please do not delete the comment, Thank You.


test branch mark
ab -n 10 -c 5 http://127.0.0.1:8080/

Result:
This is ApacheBench, Version 2.3 <$Revision: 1913912 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient).....done


Server Software:        
Server Hostname:        127.0.0.1
Server Port:            8080

Document Path:          /
Document Length:        23 bytes

Concurrency Level:      5
Time taken for tests:   0.164 seconds
Complete requests:      10
Failed requests:        0
Total transferred:      880 bytes
HTML transferred:       230 bytes
Requests per second:    60.82 [#/sec] (mean)
Time per request:       82.211 [ms] (mean)
Time per request:       16.442 [ms] (mean, across all concurrent requests)
Transfer rate:          5.23 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       0
Processing:    33   58  13.0     64      76
Waiting:       29   49  10.7     51      67
Total:         34   59  12.9     64      76

Percentage of the requests served within a certain time (ms)
  50%     64
  66%     65
  75%     67
  80%     68
  90%     76
  95%     76
  98%     76
  99%     76
 100%     76 (longest request)