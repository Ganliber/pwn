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

动态调试

```python
cyclic <- hexToDecimal(0x80-0x00) + 8 = 132 bytes

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





## pwntools技巧

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





