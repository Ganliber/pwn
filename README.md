# pwn

[TOC]



> A record of the learning trajectory of a pwn rookie🐣
## Abstract

***
* Tool `libelf`.
* Website for CTF : [picoctf (CMU)](https://picoctf.org/)



## VM-Ubuntu共享主机Clash

1. NAT模式

2. Clash -> General -> Allow LAN

3. 将鼠标停留在`Allow LAN`上可以查看ip地址（或cmd用ipconfig查看）

   ```
   以太网适配器 VMware Network Adapter VMnet8:
   
     连接特定的 DNS 后缀 . . . . . . . :
     本地链接 IPv6 地址. . . . . . . . : fe80::45f7:6280:998f:9633%3
     IPv4 地址 . . . . . . . . . . . . : 192.168.234.1
     子网掩码  . . . . . . . . . . . . : 255.255.255.0
     默认网关. . . . . . . . . . . . . :
   ```

4. ubuntu中：

   network -> VPN -> Network Proxy -> 设置 -> Manual -> 将下列所有地址改为上以太网适配器的 IPv4 地址

   -> 端口与Clash一致`7890`

5. 登录谷歌查看即可！！





## Environment

***

> 摘自CSDN博客：[Pwn环境配置: ubuntu环境搭建](https://blog.csdn.net/Y_peak/article/details/120712904)
>
> 知乎博客：[zsh & oh-my-zsh 的配置与使用 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/58073103)
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

安装`zsh`以及相应的`Oh my zsh`

> 优秀的终端
>
> 事实上如果我们要是自己配置zsh的配置的话会比较麻烦，好在github上有大佬制作了一个配置文件，“oh-my-zsh”，这是目前zsh中最流行的一个配置了。

```bash
apt install zsh #安装zsh

chsh -s /bin/zsh #将zsh设置成默认shell（不设置的话启动zsh只有直接zsh命令即可）
```

安装Oh my zsh

<img src="C:\Users\XiZhongKuiYue\AppData\Roaming\Typora\typora-user-images\image-20220531222744311.png" alt="image-20220531222744311" style="zoom:33%;" />

### oh-my-zsh配置

> ###### 1.查看什么Theme可以用
>
> ```shell
>    $ ls ~/.oh-my-zsh/themes
> ```
>
> ###### 2.查看是否有 ~/.zshrc文件，如果想要备份系统的zsh配置
>
> ```shell
>    $ cp ~/.zshrc ~/.zshrc.orig
> ```
>
> ###### 3.创建配置文件(cp 源文件 目标文件 把源文件复制到目标文件并改名，如果不存在，新建，如果已存在，内容覆盖，也可以手动)
>
> ```awk
>    $ cp ~/.oh-my-zsh/templates/zshrc.zsh-template ~/.zshrc
> ```
>
> ###### 4.Oh-My-Zsh的默认配置文件在：~/.zshrc。编辑~/.zshrc修改主题，默认情况下，使用的是robbyrussell主题：（在line 10，重启终端后有效或者使用source ~/.zshrc更新配置）
>
> ```ini
>    ZSH_THEME="amuse"
> ```



更改默认shell

```bash
chsh -s /bin/zsh
```

ys主题好看捏

> `vim ~/.zshrc`修改

```python
ZSH_THEME="ys"
```

在`~/.zshrc`中安装插件`zsh-syntax-highlighting`

> 该插件官网:[zsh-users/zsh-syntax-highlighting: Fish shell like syntax highlighting for Zsh. (github.com)](https://github.com/zsh-users/zsh-syntax-highlighting)
>
> oh-my-zsh 的自带插件都储存在 "~/.oh-my-zsh/plugins" 目录中，如果你希望安装一个插件，可以在 "~/.zshrc" 的 plugins=(xxx, xxx, ...) 这一行里加入插件名称

```
plugins=(git zsh-syntax-highlighting)
```

克隆项目

```
git clone https://github.com/zsh-users/zsh-syntax-highlighting.git ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-syntax-highlighting
复制代码
```

在 `~/.zshrc` 中配置

```
plugins=(其他的插件 zsh-syntax-highlighting)
复制代码
```

使配置生效

```
source ~/.zshrc
```

如果github出现无法访问443错误

```
git config --global http.proxy
```

使用该行命令即可



#### zsh-autosuggestions

> 自动补全
>
> 只需输入部分命令即可根据之前输入过的命令提示，按右键→即可补全

安装

```awk
git clone https://github.com/zsh-users/zsh-autosuggestions ${ZSH_CUSTOM:-~/.oh-my-zsh/custom}/plugins/zsh-autosuggestions
```

在 `~/.zshrc` 中配置

```javascript
plugins=(其他的插件 zsh-autosuggestions)
```

更新配置后重启

```bash
source ~/.zshrc
```



#### colored-man-pages

> 给你带颜色的 man 命令。
>
> 这个是oh-my-zsh自带的,我的是在：
>
> `/home/ganliber/.oh-my-zsh/plugins/colored-man-pages`

将其加入`~/.zshrc`中即可



#### autojump

> **只需要按一个`j`就可以快速跳转到目标目录**（之前进入过的目录）

安装：

```bash
git clone https://github.com/wting/autojump.gitCopy
```

还需要额外配置一下，进入克隆下来的仓库目录，执行（你得先装python）：

```bash
./install.pyCopy
```

最后把以下代码加入到`.zshrc`：

```bash
[[ -s ~/.autojump/etc/profile.d/autojump.sh ]] && . ~/.autojump/etc/profile.d/autojump.shCopy
```

**使用**：

```bash
j dirname		# 注意不是路径名而是文件夹名Copy
```

*ps：这里的`dirname`不用写全也可以（可以只是一部分），tql！😈*

删除无效路径：

```bash
j --purge 无效路径
```

* Bug

* **Problem scenario**:

  ```py
  /usr/bin/env: ‘python’: No such file or directory
  ```

  **Possible Solution #1**

  - If Python 3 is not installed, install it: `apt-get install python3`

  **Possible Solution #2**

  - If Python 3 has been installed, run these commands: `whereis python3`
  - Then we create a symlink to it: `sudo ln -s /usr/bin/python3 /usr/bin/python`

* 注意这个`j`跳转指令不依赖完整路径只适用于之前已经访问过的目录，并不适用于未访问过的目录！



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

** not reliable any more! ** 安装LibcSearcher

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
                            --- ruby 是一个包管理器 ---
                            --- 日本人开发的脚本语言 ---
sudo gem install one_gadget --- 需要翻墙, 国内互联网是真的垃圾
                            --- 这就是: zgzf对互联网的污染...
```

用法示例如下所示

<img src="C:\Users\XiZhongKuiYue\AppData\Roaming\Typora\typora-user-images\image-20220601003206268.png" alt="image-20220601003206268" style="zoom:50%;" />







安装seccomp-tools

> 这是一个沙盒，后面学的深得话会用到

```bash
sudo gem install seccomp-tools
```



安装libc-database

> 一个非常好用查找libc版本的git项目, 其实就是https://libc.blukat.me/这个网站里面的数据
>
> 注意这个网站更靠谱，而这个库不再可靠！！！

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
>
> （目前还没装）

```bash
git clone https://github.com/bash-c/main_arena_offset.git
```



### 64-bit ubuntu 运行32位程序

安装32位运行环境

```csharp
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install zlib1g:i386 libstdc++6:i386 libc6:i386
sudo apt-get install lib32z1
```



### 修改ELF文件libc为指定版本

遇到本地libc与题目libc不匹配问题，导致一些套路无法在本地调试和利用。要想gdb,不仅得要安个符合版本的虚拟机或起一个docker还有部署一边pwn环境.于是想找下有没有更方便的方法. 于是找到了patchelf更换libc的方法。

#### glibc-all-in-one与patchelf安装

glibc-all-in-one，正如其名是一个**多版本libc的下载安装管理工具**，主要支持2.19，2.23-2.29版本的libc和i686, amd64的架构。这是github一个开源项目因此我们git它既可。

安装命令：

```
git clone https://github.com/matrix1001/glibc-all-in-one.git 
cd glibc-all-in-one 
chmod a+x build download extract
```

patchelf在ubuntu直接`apt install patchelf`即可。





#### 下载对应的ld

进入`glibc-all-in-one`文件夹下，运行脚本`update_list`，然后`cat list`查看对应版本的ld

```shell
./update_list
cat list
2.23-0ubuntu11.3_amd64
2.23-0ubuntu11.3_i386
2.23-0ubuntu3_amd64
2.23-0ubuntu3_i386
2.27-3ubuntu1.5_amd64

***********************

$ ./libc-2.23.so    
GNU C Library (Ubuntu GLIBC 2.23-0ubuntu11) stable release version 2.23, by Roland McGrath et al.
```

注意先通过执行对应的`libc`查看适配的版本（ubuntu版本+libc版本）

执行脚本`download`

```shell
./download 2.23-0ubuntu11.3_i386
```

下载好的`glibc`在lib文件夹中

```shell
$ cd 2.23-0ubuntu11.3_i386
$ ls
ld-2.23.so               libcidn.so.1      libm.so.6              libnss_files.so.2       libpthread-2.23.so   libthread_db.so.1
ld-linux.so.2            libcrypt-2.23.so  libnsl-2.23.so         libnss_hesiod-2.23.so   libpthread.so.0      libutil-2.23.so
libanl-2.23.so           libcrypt.so.1     libnsl.so.1            libnss_hesiod.so.2      libresolv-2.23.so    libutil.so.1
libanl.so.1              libc.so.6         libnss_compat-2.23.so  libnss_nis-2.23.so      libresolv.so.2
libBrokenLocale-2.23.so  libdl-2.23.so     libnss_compat.so.2     libnss_nisplus-2.23.so  librt-2.23.so
libBrokenLocale.so.1     libdl.so.2        libnss_dns-2.23.so     libnss_nisplus.so.2     librt.so.1
libc-2.23.so             libm-2.23.so      libnss_dns.so.2        libnss_nis.so.2         libSegFault.so
libcidn-2.23.so          libmemusage.so    libnss_files-2.23.so   libpcprofile.so         libthread_db-1.0.so
```

然后复制ld文件到pwn题目录下

```shell
cp ./ld-2.23.so ~/Documents/ROP/ret2libc3
```

更改文件的`ld`和`libc`

```shell
patchelf --replace-needed /lib/i386-linux-gnu/libc.so.6 ./libc-2.23.so ./ret2libc3
(or) patchelf --replace-needed libc.so.6 ./libc-2.23.so ./ret2libc3
---------------------------------------------------------------------- 原本的libc版本通过ldd查看
patchelf --set-interpreter ./ld-2.23.so ./ret2libc3
----------------------------------------------------------------------
./ret2libc3 --> 执行查看
```

顺便提一嘴定向查找历史指令

```shell
history | grep patchelf
```





#### 对应libc编译

我们可以通过在glibc-all-in-one目录下执行`./build`即可获对应版本的libc和ld.so

例如：`./build 2.29 i686`

下载安装编译 32位的2.29 版本libc。

#### patchelf更改程序libc

* 修改`ld.so`：：：：：：执行`patchelf --set-interpreter ld.so elf` 来修改文件`ld.so`
* 修改`libc.so`：：：：：执行`patchelf --replace-needed old_libc.so new_libc.so elf`来修改文件`libc.so`



### 解析并更改ELF文件的依赖库

> 详见博客：[pwn更换目标程序libc_pwn切换libc](https://blog.csdn.net/yongbaoii/article/details/111938821?ops_request_misc=%7B%22request%5Fid%22%3A%22165435003416781667876674%22%2C%22scm%22%3A%2220140713.130102334..%22%7D&request_id=165435003416781667876674&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-2-111938821-null-null.142^v11^pc_search_result_control_group,157^v13^control&utm_term=patchelf如何更换libc&spm=1018.2226.3001.4187)

`readelf -h`查看帮助

```bash
-d --dynamic           Display the dynamic section (if present)
```

因此使用

```bash
readelf -d my-program
```

可以查看动态节的内容

使用`ldd`命令查看某ELF文件依赖的`libc`库文件的路径

```bash
# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:39:29] 
$ ls  
libc-2.23.so  ret2libc3
# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:41:14] 
$ ldd ret2libc3
	linux-gate.so.1 (0xf7f01000)
	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xf7cf9000)
	/lib/ld-linux.so.2 (0xf7f03000)
```

然后判断你要换的libc的版本，因为`libc`的版本跟`ld`的版本是要匹配的，比如这里我要换的是libc_32.so.6，我查看他的版本是2.23，所以你要把ld换成2.23的。

怎么查看libc版本，直接拖到shell里面跑一下就好，如（2.23）

```bash
# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:47:04] 
$ ls              
libc-2.23.so  ret2libc3
# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:47:05] 
$ ./libc-2.23.so
GNU C Library (Ubuntu GLIBC 2.23-0ubuntu11) stable release version 2.23, by Roland McGrath et al.
Copyright (C) 2016 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.
Compiled by GNU CC version 5.4.0 20160609.
Available extensions:
	crypt add-on version 2.1 by Michael Glad and others
	GNU Libidn by Simon Josefsson
	Native POSIX Threads Library by Ulrich Drepper et al
	BIND-8.2.3-T5B
libc ABIs: UNIQUE IFUNC
For bug reporting instructions, please see:
<https://bugs.launchpad.net/ubuntu/+source/glibc/+bugs>.
```

查看`ELF`文件原来所用的`libc`版本（2.31）

```shell
# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:47:13] 
$ ldd ret2libc3   
	linux-gate.so.1 (0xf7f96000)
	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xf7d8e000)
	/lib/ld-linux.so.2 (0xf7f98000)

# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:48:07] 
$ /lib/i386-linux-gnu/libc.so.6
GNU C Library (Ubuntu GLIBC 2.31-0ubuntu9.9) stable release version 2.31.
Copyright (C) 2020 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.
Compiled by GNU CC version 9.4.0.
libc ABIs: UNIQUE IFUNC ABSOLUTE
For bug reporting instructions, please see:
<https://bugs.launchpad.net/ubuntu/+source/glibc/+bugs>.
```



## 安装LibcSearcher(X)

> **！！！LibcSearcher已经不能用了，就自己搭建本地`libc-database`使用`./add ./find ./dump`三剑客即可！！！**
>
> 官方网址：https://pypi.org/project/LibcSearcher/

```
在练习Pwn过程中，要用到python的一个库，叫做LibcSearcher，安装方法如下：
> 新版的LibcSearcher
1. pip3 install LibcSearcher (不要直接 clone !!!)
2. alias python=python3 (设置python默认为py3)
```

<img src="D:\github\pwn\pwn_study\images\libcSearcher_new_py3.png" alt="libcSearcher_new_py3" style="zoom:50%;" />

> 只需要联网使用即可

<img src="C:\Users\XiZhongKuiYue\AppData\Roaming\Typora\typora-user-images\image-20220606004831069.png" alt="image-20220606004831069" style="zoom: 50%;" />



### LibcSearcher 配合本地 libc-database

本地安装`LibcSearcher`（非联网版本）

> 原仓库地址https://github.com/lieanu/LibcSearcher

```python
git clone https://github.com/lieanu/LibcSearcher.git
cd LibcSearcher
python setup.py develop
```

使用

```python
from LibcSearcher import *
# 第二个参数，为已泄露的实际地址,或最后12位(比如：d90)，int类型
obj = LibcSearcher("fgets", 0X7ff39014bd90)

obj.dump("system")        #system 偏移
obj.dump("str_bin_sh")    #/bin/sh 偏移
obj.dump("__libc_start_main_ret")    
```

进入下载的文件夹下

删除`libc-database`

```python
rm -f -r libc-database
```

然后

```python
git clone https://github.com/niklasb/libc-database.git
cd libc-database
./get ubuntu  
注意需要查看 README.md 下载需要满足的依赖项即 Debian-based 下的各种软件工具, 可以通过[name] --version查看
```

自添加libc库（可能存在有些`libc`版本的库在远程`database`中没有）

> `./get` 遇到问题可见https://blog.csdn.net/Invin_cible/article/details/121326430

```python
./get 下载get工具, 若已下载请直接跳过

./add usr/lib/libc-2.21-so 向数据库中添加自定义 libc

./find __libc_start_main xxx 这里输入你要查找的函数的真实地址的后三位

./dump xxx 转储一些有用的偏移量，给出一个 libc ID, 这里输入第三步得到的结果中ID后的libc库。这样你就可以得到需要的文件中的偏移地址了
```

> 成功添加本地的`libc`库
>
> 注意`local-xxxxxxxxxx`是他的`ID`哦，`./dump`的时候就需要将整个ID输入到后面，这样就可以查看到相应的偏移了！！！
>
> 

<img src="D:\github\pwn\pwn_study\images\libc-database_added_custom_libc.png" alt="libc-database_added_custom_libc" style="zoom: 50%;" />







## Linux 关闭 ASLR

ALSR由 `/proc/sys/kernel/randomize_va_space` 决定，默认为2

> 0 - 表示关闭进程地址空间随机化
>
> 1 - 表示将mmap的基址，stack和vdso页面随机化。
>
> 2 - 表示在1的基础上增加栈（heap）的随机化。

要关闭ALSR，只需将`randomize_va_space`里面的内容改为0即可。

修改该文件不能直接用 `vi` 或者 `vim` 进行修改,也不能直接 `sudo echo 0 > /proc/sys/kernel/randomize_va_space`

使用此条命令即可关闭ALSR：`sudo sh -c "echo 0 > /proc/sys/kernel/randomize_va_space"`

查看系统ASLR是否开启

```shell
$ cat /proc/sys/kernel/randomize_va_space     
2
```





## 关于Ubuntu高版本`system`调用失败

> 由于是在64位机器上运行32位程序，因此需要考虑`rsp`的`16-bit`对齐问题

* Ubuntu18.04 64位 和 部分Ubuntu16.04 64位 调用system的时候，**rsp的最低字节必须为0x00（栈以16字节对齐）**，否则无法运行system指令。要解决这个问题，**只要将返回地址设置为跳过函数开头的push rbp就可以了**
* 栈的字节对齐，实际是指栈顶指针必须须是16字节的整数倍。我们都知道栈对齐帮助在尽可能少的内存访问周期内读取数据，不对齐堆栈指针可能导致严重的性能下降。





## 补充

> 补充一些小技巧

```python
 info proc map 查看各个库加载信息然后寻找 "/bin/sh" 字符串
 strings: 查看文件中可见字符串
 strings -a -t x /lib32/libc.so.6 | grep "/bin/sh"
 objdump -d file | grep "ret" 可以用来查找ret指令
 objdump -x [filename] 打印头文件信息以及区段信息
 objdump -T libc.so | grep gets
```
