#一个web服务器并发测试工具Damocles

##说明：
达摩克利斯剑，是一个使用C语言编写的测试web服务器负载能力的工具，参考了经典的workbench实现。通过这个项目有利于理解http服务流程，以及多线程调试，信号，如何编写makefile等诸多知识点。

##使用方法：
1. 平台：centos7 编译器：gcc 4.8.4
2. 下载项目
3. cd Damocles && make
4. 运行./damo -h 可以查看帮助信息
![工具选项](https://github.com/hunterzhao/Damocles/blob/master/pic/option.png?raw=true)

##例子：
使用两个线程，3秒内不断的访问www.baidu.com

`./damo -c 2 -t 3 -p 80 www.baidu.com/`

输出结果包括访问的速度，访问成功与失败的次数

![结果](https://github.com/hunterzhao/Damocles/blob/master/pic/result.png?raw=true)

##输出说明

- Copyright 版本说明
- Total time 总运行时间
- Speed 平均访问速度，包括pages/min MB/sec两种
- Requests 请求的成功与失败次数
-  the shortest time to open the webpage 打开网页最快时间
- the longest time to open the webpage 打开网页最慢时间

##注意：
socket进程通信中，千万要小心close的使用，尤其是大量的数据传输时，很有可能导致接收方无法获得完整数据

##感谢：
**谢谢您的查看，水平有限献丑了,如果对您有帮助请给我点star哦 :)**

##后续：
1. 支持分布式多主机同时对一台http服务器进行压力测试（有点DDoS的感觉）
2. 一直想在黑框框里做一个进度条

##参考：
1. [http 请求报文 与 响应报文][1]
2. [GDB 调试多进程方法][2]
3. [GETOPT 参数设置函数][3]

[1]: http://network.chinabyte.com/401/13238901.shtml
[2]: http://blog.csdn.net/pbymw8iwm/article/details/7876797
[3]: http://man7.org/linux/man-pages/man3/getopt.3.html

