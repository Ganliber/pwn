# pwn

[TOC]



> A record of the learning trajectory of a pwn rookie🐣
### Abstract
***
* Tool `libelf`.
* Website for CTF : [picoctf (CMU)](https://picoctf.org/)



### VM-Ubuntu共享主机Clash

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





### Environment

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

oh-my=zsh配置

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

