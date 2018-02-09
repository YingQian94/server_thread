# server_thread
采用Reactor模型实现的线程池版图像服务器

1.使用Reactor模型（non-blocking IO+epoll）处理高并发请求
2.添加线程池处理图片计算任务，线程池采用队列实现
3.使用RAII管理mutex资源
4.添加定时器丢弃非活动连接
  定时器采用最小堆结构实现，使得选择最小时间的复杂度为O(1),调整最小堆的时间为O（logN)
5.添加读任务队列，计算任务队列，写任务队列，使得主线程能够高效地处理连接需求
6.添加日志，记录服务器成功及错误信息