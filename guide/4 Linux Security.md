# Linux 安全机制

[TOC]

## linux 基础

##### 流(stream)，管道(pipeline)，重定向(relocation)

* linux系统一切可视为文件，**标准流**定义在头文件`unistd.h`中

  | 文件描述符 | stdio流 |
  | ---------- | ------- |
  | 0          | stdin   |
  | 1          | stdout  |
  | 2          | stderr  |

* **管道**`pipeline` : 一系列进程`process`通过标准流连接在了一起，前一个进程的输出`stdout`直接作为后一个进程的输入`stdin`，管道符号为`|`。示例：

  ```shell
  ps -aux | grep bash
  ```

##### 权限与用户

* UID 和 GID , GID 的关系储存在 `/etc/group` 中

* 所有用户信息（除了密码）保存在`/etc/passwd`中，加密的密码保存在`/etc/shadow`(只有`root`可以访问)中

  ```shell
  ls -l [file]
  ```

  详细查看文件类型权限等信息

##### 环境变量

* 相当于给系统或应用程序设置了一些参数，如共享库的位置，命令行的参数等信息，对程序运行十分重要

* 存在形式

  ```shell
  name=value
  ```

  * name(环境变量名): 由大写字母和下划线组成
  * value(环境变量值): 需要以`\0`结尾
  * 分类 1 (生命周期)
    * 永久环境变量：修改相关配置文件，永久生效
    * 临时环境变量：`export`在当前终端声明，关闭终端后失效

  * 分类 2 (作用域)
    * 系统环境变量：对所有用户生效，配置于`/etc/profile`
    * 用户环境变量：对特定用户生效，配置于`~/.bashrc`

* 查看

  ```shell
  env [option]...
  ```

* `LP_PRELOAD`字段

  定义程序运行时优先加载的动态链接库

* `environ`字段

  `libc`中定义的全局变量`environ`指向内存中的环境变量表

##### procfs 文件系统

* `procfs file system`是linux内核提供的**虚拟文件系统**（只占内存不占存储），为访问内核数据提供接口。
* 每个正在运行的进程都对应`/proc`下的一个目录，目录名即是进程的`PID`
* 

































