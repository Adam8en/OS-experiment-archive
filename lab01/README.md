1.c、2.c、3.c对应1、2、3问的代码

q1、q2、q3对应1、2、3问代码生成的可执行文件

1.1.c和q1.1是拓展部分的代码和可执行文件

test和test1是用于测试1.c的可执行文件，运行结果如下图所示：

![image-20241010225437071](https://adam8en-blog-image.oss-cn-guangzhou.aliyuncs.com/image-20241010225437071.png)

执行以下命令来编译代码生成可执行文件：

~~~bash
gcc 1.c -o q1 -lpthread
gcc 2.c -o q2 -lpthread
gcc 3.c -o q3 -lpthread
gcc 1.1.c -o q1.1 -lpthread
~~~

