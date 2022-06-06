# Tips

[TOC]



## libc database search

> 使用方法

- 在左侧的框中填入最低12位（在gdb调试中`got`查看或者远程用内存泄漏查找），即后3*4位
- 选择一个libc
- 使用下面框中的偏移进行计算



### 查看已知`libc`版本

> `strings [libc文件] | grep GNU`

```python
$ strings ./libc-2.23.so |grep GNU
GNU C Library (Ubuntu GLIBC 2.23-0ubuntu11) stable release version 2.23, by Roland McGrath et al.
Compiled by GNU CC version 5.4.0 20160609.
	GNU Libidn by Simon Josefsson
```



## libc-database 本地部署





### libc_database的使用方法

> 在此默认你已经下载了libc_database

```python
./get 下载get工具, 若已下载请直接跳过

./add usr/lib/libc-2.21-so 向数据库中添加自定义 libc

./find __libc_start_main xxx 这里输入你要查找的函数的真实地址的后三位

./dump xxx 转储一些有用的偏移量，给出一个 libc ID, 这里输入第三步得到的结果中ID后的libc库。这样你就可以得到需要的文件中的偏移地址了
```



















