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



##### 调用约定

* `linux interface`
  * x86-32 : 
    * eax <-- syscall_number
    * ebx, ecx, edx, esi, ebp 用于将6个参数传递给系统调用
  * x86-64 :
    * rax <-- syscall_number
    * rdi, rsi, rdx,  r10, r8, r9 (by syscall)

* `user interface`
  * x86-32 : stack
  * x86-64 : rdi, rsi, rdx, rcx, r8, r9 (注意如果参数类型是`memory`类型，则在栈上传递参数)
* `system call`
  * `int 0x80` : 早期2.6以及更早版本的kernel
  * `syscenter` : 32-bit system (For using syscenter, you have to manually lay out stacks)
  * `syscall` : 64-bit system

* `ld`instruction : GNU的连接器，用于将目标文件连接为可执行程序

  ```shell
  ld [options] objfile ...
  + -o : 指定输出文件名
  + -e : 指定程序入口符号
  ```

* `strace` intruction : 按照strace官网的描述, strace是一个可用于诊断、调试和教学的Linux用户空间跟踪器。我们用它来监控用户空间进程和内核的交互，比如系统调用、信号传递、进程状态变更等。





## Core Dump

| Signal  | Action | Explanation          |
| ------- | ------ | -------------------- |
| SIGQUIT | Core   | 键盘退出             |
| SIGILL  | Core   | 非法指令             |
| SIGABRT | Core   | 从about产生的信号    |
| SIGSEGV | Core   | 无效内存访问         |
| SIGTRAP | Core   | trace/breakpoint陷阱 |

查看`core dump`机制是否开启

```shell
ulimit -c
```

临时开启

```shell
ulimit -c unlimited
```

永久开启 ：查看配置文件

```shell
cat /etc/security/limits.conf

+ ------------------- result ------------------ +
+ <domain>      <type>       <item>    <value>  +
+	*              soft         core        0     +
+ --------------------------------------------- +
```

将value改为unlimited即可

将核心转储`core dump`文件文件名变为`core.[pid]`储存

```shell
echo 1 > /proc/sys/kernel/core_uses_pid
```

修改`core dump`的默认存储位置：修改`core_pattern`，保存到`/tmp`目录，文件名为`core-[filename]-[pid]-[time]`

```shell
echo /tmp/core-%e-%p-%t > /proc/sys/kernel/core_pattern
```





## Stack Canaries

> Canary是栈上的一个随机数，程序启动时随机生成并保存在比函数 return address 更低的位置。

##### Classification

###### Classification generally

* terminator : 将低位设置为`\x00(string ending)`，防止泄露，截断字符还包括：CR(0x0d), LF(0x0a), EOF(0xff)
* random
* random XOR

###### Implementation

* StackGuard (1997)
* StackShield
* ProPoliced

###### code

```shell
++++++ Canary ++++++
+ -fstack-protector
+ -fstack-protector-stronge
+ -fstack-protector-all 
+ -fstack-protector-explicit
+ -fno-stack-protector : disable protection
```

对于程序`canary.c`

```C
#include<stdio.h>
void main()
{
  char buf[10];
  scanf("%s", buf);
}
```

编译 1 (取消保护机制)

```shell
gcc -fno-stack-protector canary.c -o fno.out
```

编译 2 (开启保护机制)

```assembly
(gdb) disas main
Dump of assembler code for function main:
0x0000000000001169 <+0>:     endbr64
0x000000000000116d <+4>:     push   %rbp
0x000000000000116e <+5>:     mov    %rsp,%rbp
0x0000000000001171 <+8>:     sub    $0x20,%rsp
0x0000000000001175 <+12>:    mov    %fs:0x28,%rax
0x000000000000117e <+21>:    mov    %rax,-0x8(%rbp)
0x0000000000001182 <+25>:    xor    %eax,%eax
0x0000000000001184 <+27>:    lea    -0x12(%rbp),%rax
0x0000000000001188 <+31>:    mov    %rax,%rsi
0x000000000000118b <+34>:    lea    0xe72(%rip),%rdi        # 0x2004
0x0000000000001192 <+41>:    mov    $0x0,%eax
0x0000000000001197 <+46>:    callq  0x1070 <__isoc99_scanf@plt>
0x000000000000119c <+51>:    nop
0x000000000000119d <+52>:    mov    -0x8(%rbp),%rax
0x00000000000011a1 <+56>:    xor    %fs:0x28,%rax
0x00000000000011aa <+65>:    je     0x11b1 <main+72>
0x00000000000011ac <+67>:    callq  0x1060 <__stack_chk_fail@plt>
0x00000000000011b1 <+72>:    leaveq
0x00000000000011b2 <+73>:    retq
End of assembler dump.
```

注意到<+12>和<+56>处的指令

* `fs` register `%fs:0x28`: 用于存放 **线程局部存储** `Thread Local Storage,TLS` ,TLS主要用于避免多个线程同时访存同一全局变量`global variable`或者静态变量`static variable`时所导致的冲突，***尤其当多个线程同时修改该变量时.***
* `gs` register `%gs:0x14`: Canary of 32-bit program. 

















