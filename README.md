# pwn

[TOC]



> A record of the learning trajectory of a pwn rookie🐣
### Abstract
***
* Tool `libelf`.
* Website for CTF : [picoctf (CMU)](https://picoctf.org/)





### Environment

***

> 摘自CSDN博客：[Pwn环境配置: ubuntu环境搭建](https://blog.csdn.net/Y_peak/article/details/120712904)
>
> 本教程是针对ubuntu20.04的版本，下载链接ubuntu20.04官方链接放心使用。不过不知道后面官方链接这个是否会更新。更新了就自己找找吧。

```bash
List:
gcc, vim, git, pip3, 
pwntools, checksec, pwndbg, one_gadget
ipython, LibcSearcher, ROPgadet, main_arena_offset
```

切换下载源(阿里源)

```bash
# 备份源
sudo cp /etc/apt/sources.list  /etc/apt/sources.list_backup
# 编辑源
sudo vi /etc/apt/sources.list
```

进入`/etc/apt/sources.list`之后更换注释掉原内容

```bash
deb http://mirrors.aliyun.com/ubuntu/ focal main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ focal main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ focal-security main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ focal-security main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ focal-updates main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ focal-updates main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ focal-proposed main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ focal-proposed main restricted universe multiverse
deb http://mirrors.aliyun.com/ubuntu/ focal-backports main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu/ focal-backports main restricted universe multiverse
```

更新源

```text
sudo apt update
```

更换`shell`

查看系统有几种shell

```bash
cat /etc/shells 
```



安装gcc

```bash
sudo apt install gcc
```

安装pip

> 这里安装的是`python3`的pip，毕竟python2早就已经停产了。

```bash
sudo apt install python3-pip
```

安装pwntools

```bash 
pip install pwntools
```


如果嫌弃慢可以pip换源一下

```bash 
pip install pwntools -i https://pypi.tuna.tsinghua.edu.cn/simple/ 
```

将pip以及相关的包加入CLI默认路径

>`ganliber`应该换成你的用户名

```bash
echo "export PATH=\"/home/ganliber/.local/bin:\$PATH\"" >> ~/.bashrc && source ~/.bashrc
```

安装vim编辑器

```bash 
sudo apt install vim
```

安装git

> 这个必须安装，安装过后就可以从github上面下载东西了。

```bash
sudo apt install git
```

安装checksec

> 一个辅助工具，可以用来检测二进制文件格式和安全防护情况

```bash 
sudo apt install checksec
```

安装pwndbg（需要先安装gdb，pwndbg只是一个插件，这样执行`gdb`之后就可以直接显示为`>pwndbg`了）

> 这个也是需要的，毕竟gdb在没有插件的时候对用户实在是不怎么友好。当然你喜欢gef，或者peda也随意。反正我只用过这个

```bash 
git clone https://github.com/pwndbg/pwndbg
cd pwndbg
./setup.sh
```

peda的

```bash
git clone https://github.com/longld/peda.git ~/peda
echo "source ~/peda/peda.py" >> ~/.gdbinit
```

安装ipython

> 这个不是必须的，但是还是挺好用的。你用过就知道了，相当于python的输出加强版

```bash
pip install ipython
```

安装LibcSearcher

> LibcSearcher是针对pwn做的python库，在做pwn题时寻找一些libc版本非常好用，不过好像现在有点疲软了。毕竟这个是对于python2写的，而且好久不维护了。开始的时候用着还可以，现在不大行了。想用还是可以用一下的。

```bash 
git clone https://github.com/lieanu/LibcSearcher.git
cd LibcSearcher
python setup.py develop
```

安装ROPgadget

> 便于你构造rop链的工具，不过好像安装pwntools之后会自动安装这个玩意。要是没有你就自己安装吧。输入Ropgadget验证一下就好

```bash
sudo pip install capstone
pip install ropgadget
ROPgadget 
```

安装one_gadget

> 也是构造rop链用的

```bash
sudo apt install ruby
sudo gem install one_gadget
```

安装seccomp-tools

> 这是一个沙盒，后面学的深得话会用到

```bash
sudo gem install seccomp-tools
```

安装libc-database

> 一个非常好用查找libc版本的git项目, 其实就是https://libc.blukat.me/这个网站里面的数据

```bash
#下载项目
git clone https://github.com/niklasb/libc-database.git
# 建立一个libc偏移量数据库 
./get  # List categories
./get ubuntu debian  # Download Ubuntu's and Debian's libc, old default behavior
./get all  # Download all categories. Can take a while!
```

安装main_arene_offset

> 据说做堆题的话，比较好用，还没用过呢。菜鸡一个没怎么写过堆。感觉有一丢丢难

```bash
git clone https://github.com/bash-c/main_arena_offset.git
```

