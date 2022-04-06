# 二进制文件

**大纲**

[TOC]

## 编译原理

* 编译工具

  * `cc1` : 编译器 (Preprocess`hello.i`, Compile`hello.s`) 
  * `as` : 汇编器 (Assemble`hello.o`)
  * `collect2` : 链接器 (Link`hello`)

* 编译过程

  * `Preprocess`

    ```shell
    gcc -E hello.c -o helo.i
    ```
    
  * `Compile`
  
    ```shell
    gcc -S hello.c -o hello.s
    gcc -S hello.i -o hello.s
    ```
  
  * `Assembly`
  
    ```shell
    gcc -c hello.c -o hello.o
    gcc -c hello.s -o hello.o  # hello.o is a relocatable file
    ```
  
  * `Link`
  
    > 1.  gcc 默认使用动态链接
    >
    > 2. 目标文件及其依赖库进行链接，生成可执行文件
    > 3. Address and Storage Allocation, Symbol binding, Relocation
  
    ```shell
    gcc hello.o -o hello -static  # 如果没有-static则是动态链接
    ```



## ELF 文件格式

##### 预备知识

* Executable and Linkable Format : Unix Lab 作为二进制接口 (Application Binary Interface - `ABI`) 使用
* Linux系统运行的就是ELF格式的文件, 相关定义在 `/usr/include/elf.h` 中
* File Classification
  * 可执行文件 `.exec`
  * 可重定位文件 `.rel` : 常常以`.o`为文件后缀名，用于与其他目标文件进行链接以构成**可执行文件**或**动态链接库**，常为一段位置独立代码 (Position Independent Code, `PIC`)
  * 共享目标文件 `.dyn` : **动态链接库文件**
  * 核心转储文件 `Core Dump file` : 进程意外终止时进程地址空间的转储, 也是`ELF`文件的一种。使用`GDB`读取这类文件可以辅助调试和查找程序崩溃的原因
* File Structure
  * `ELF`统称为`Object file` (与`.o` (统称为**可重定位文件**) 不同)
  * 视角
    * 链接视角(based on **Section**)
      * `ELF Header` : 文件头
      * `.text` : 可执行机器指令
      * `.data` : **已初始化**的：全局变量, 局部静态变量
      * `.bss` : **未初始化**的：全局变量, 局部静态变量
    * 运行视角(based on **Segment**)

##### ELF Header

* 参考 : [ELF Header](https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/chapter6-43405/index.html)

* The ELF header has the following structure. See `sys/elf.h`.

  ```C
  typedef struct
  {
    unsigned char    e_ident[EI_NIDENT];    /* Magic number and other info */
    Elf32_Half    e_type;            /* Object file type */
    Elf32_Half    e_machine;        /* Architecture */
    Elf32_Word    e_version;        /* Object file version */
    Elf32_Addr    e_entry;        /* Entry point virtual address */
    Elf32_Off    e_phoff;        /* Program header table file offset */
    Elf32_Off    e_shoff;        /* Section header table file offset */
    Elf32_Word    e_flags;        /* Processor-specific flags */
    Elf32_Half    e_ehsize;        /* ELF header size in bytes */
    Elf32_Half    e_phentsize;        /* Program header table entry size */
    Elf32_Half    e_phnum;        /* Program header table entry count */
    Elf32_Half    e_shentsize;        /* Section header table entry size */
    Elf32_Half    e_shnum;        /* Section header table entry count */
    Elf32_Half    e_shstrndx;        /* Section header string table index */
  } Elf32_Ehdr;
  
  typedef struct
  {
    unsigned char    e_ident[EI_NIDENT];    /* Magic number and other info */
    Elf64_Half    e_type;            /* Object file type */
    Elf64_Half    e_machine;        /* Architecture */
    Elf64_Word    e_version;        /* Object file version */
    Elf64_Addr    e_entry;        /* Entry point virtual address */
    Elf64_Off    e_phoff;        /* Program header table file offset */
    Elf64_Off    e_shoff;        /* Section header table file offset */
    Elf64_Word    e_flags;        /* Processor-specific flags */
    Elf64_Half    e_ehsize;        /* ELF header size in bytes */
    Elf64_Half    e_phentsize;        /* Program header table entry size */
    Elf64_Half    e_phnum;        /* Program header table entry count */
    Elf64_Half    e_shentsize;        /* Section header table entry size */
    Elf64_Half    e_shnum;        /* Section header table entry count */
    Elf64_Half    e_shstrndx;        /* Section header string table index */
  } Elf64_Ehdr;
  
  ```

* ELF Header在`/chapter2/2.2_elfDemo.c`中

  ```shell
  gcc -c elfDemo.c -o elfDemo.rel  # relocable file
  ```

  执行查看elf表头内容

  ```shell
  readelf -h elfDemo.rel
  ```

  ELF Header Contents

  ```assembly
  ELF Header:                                                                         Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00                               Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              REL (Relocatable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x0
  Start of program headers:          0 (bytes into file)    
  Start of section headers:          1192 (bytes into file)     
  Flags:                             0x0                
  Size of this header:               64 (bytes)         
  Size of program headers:           0 (bytes)         
  Number of program headers:         0 
  Size of section headers:           64 (bytes) 
  Number of section headers:         14
  Section header string table index: 13   
  ```

##### Section Header Table

> 基于可重定向文件`elfDemo.rel`

* 参考 : [ELF](https://www.cntofu.com/book/104/Theory/ELF.md)

* 查看节头表

  ```shell
  readelf -S elfDemo.rel
  ```

* `ELF64_Shdr`

  ```C
  typedef struct elf64_shdr {
  	Elf64_Word sh_name; //节名
  	Elf64_Word sh_type; //节的类型
  	Elf64_Xword sh_flags; //节的属性
  	Elf64_Addr sh_addr;  //内存地址
  	Elf64_Off sh_offset;  //文件中的偏移
  	Elf64_Xword sh_size;  //节大小
  	Elf64_Word sh_link;  //到其他节的链接信息
  	Elf64_Word sh_info;  //各种各样的信息
  	Elf64_Xword sh_addralign;  //地址对其
  	Elf64_Xword sh_entsize;  //表项的大小
  } Elf64_Shdr;
  ```

* 对`elfDemo.rel`反汇编可以查看节表头以及汇编代码等信息

  ```shell
  objdump -x -s -d elfDemo.rel
  ```

  * -x : 显示所可用的头信息，包括符号表，重定位入口
  * -s : 显示指定section的完整内容，默认所有非空section都会被显示
  * -d : 从`objfile`中反汇编那些特定指令机器码的section
  * -h : 显示目标文件各个section的头部摘要信息
  * -r : 显示文件的重定位入口

  结果最左列的是偏移地址

  开始信息

  ```assembly
  elfDemo.rel:     file format elf64-x86-64                                                       
  elfDemo.rel                                                 
  architecture: i386:x86-64, flags 0x00000011:         
  HAS_RELOC, HAS_SYMS  
  start address 0x0000000000000000  
  ```

  各个模块

  ```assembly
  Sections:
  ...
  SYMBOL TABLE:
  ...
  Contents of section .text:
  ...
  Contents of section .data: ;
  ...
  Contents of section .rodata: ;只读数据:只读变量,字符串常量
  ...
  Contents of section .comment: ;版本控制信息,如编译器的版本
  ...
  ... ;省略若干节头信息
  Disassembly of section .text:
  ... ;汇编代码
  ```

  `.strtab`字符串表, 表示**符号名**和**节名**，引用字符串只需给出字符列表在表中的偏移。字符串表的第一个和最后一个字符都是`null`, 查看:

  ```shell
  readelf -x .strtab elfDemo.rel
  ```

  此时执行

  ```shell
  readelf -s .strtab elfDemo.rel
  ```

  可以得到目标文件中所用到的文件信息

  `.dynsym` (动态链接符号表)

  `Elf64_Sym` 结构体

  ```C
  struct Elf64_Sym
  {
    Elf64_Word    st_name;   /* Symbol name (string tbl index) */
    unsigned char st_info;   /* Symbol type and binding */
    unsigned char st_other;  /* Symbol visibility */
    Elf64_Section st_shndx;  /* Section index */
    Elf64_Addr    st_value;  /* Symbol value */
    Elf64_Xword   st_size;   /* Symbol size */
  };
  ```

##### Segment Header Table

> 可执行文件的装载：基于可执行文件`elfDemo.exec`

* 运行的视角

  ```shell
  gcc elfDemo.c -o elfDemo.exec  # 生成可执行文件
  file elfDemo.exec  # 查看文件信息
  ```

  查看结果

  ```
  elfDemo.exec: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=652894166f5d32fe0b4c62b94ecc05b8f78c1a47, for GNU/Linux 3.2.0, not stripped   
  ```

  **可执行文件**和**动态链接库**装载到进程空间中形成一个**进程镜像**，每个进程拥有独立的**虚拟地址空间**，这个空间如何布局是由段头表的程序头`Program header`决定的。ELF文件头的`e_phoff`给出了段头表的位置，查看

  ```shell 
  readelf -l elfDemo.exec
  ```

  局部结果

  ```assembly
   Section to Segment mapping:                                                                    Segment Sections... 
   00                                                             
   01     .interp                                                                          
   02     .interp .note.gnu.property .note.gnu.build-id .note.ABI-tag .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt                                     
   03     .init .plt .plt.got .plt.sec .text .fini
   04     .rodata .eh_frame_hdr .eh_frame
   05     .init_array .fini_array .dynamic .got .data .bss
   06     .dynamic
   07     .note.gnu.property
   08     .note.gnu.build-id .note.ABI-tag
   09     .note.gnu.property
   10     .eh_frame_hdr
   11
   12     .init_array .fini_array .dynamic .got
  ```

  每个Segment都包含了若干Section

  系统关心的是这些节的**权限**而非内容，因此为了减少内存映射的空间和资源浪费采取了**按不同权限的节分组**的策略。

  如`05`行`.data`和`.bss`具有`rw`权限，`03`行`.plt.got`和`.text`具有`rx`权限



## 静态链接

##### 地址空间分配

* 链接( by linker )：编译时链接( compile time )，加载时链接( load time )，运行时链接( run time )，链接时进行了地址空间分配

* `elfDemo.c`文件分成了`main.c`和`func.c`

* 静态编译

  ```shell
  gcc -static -fno-stack-protector main.c func.c -save-temps --verbose -o func.ELF
  ```

  * --verbose : （冗长的）打印出编译连接时的**详细**信息
  * -static : 静态链接
  * -fno-stack-protector : **禁用**堆栈保护
  * -fstack-protector：启用堆栈保护，不过只为局部变量中含有 char 数组的函数插入保护代码。
  * -fstack-protector-all：启用堆栈保护，为所有函数插入保护代码。
  * save-temps：gcc提供了该选项，可以保存编译过程中的**中间文件**，这里面就包括了预处理之后的源码。
  * 段的装载地址和空间以页为单位对齐

* 链接方案 ：相似`Section`合并

* 细节

  > There are two critical things `linker` need to do to build `executable file`: symbol resolution, relocation

  * 符号解析：将每个符号（函数，全局变量，静态变量）的引用和定义关联
  * 重定位：将每个符号的定义与一个内存地址相关联，修改引用使之指向内存地址

  最终链接的文件为`func.ELF`, 对main.o进行反汇编并标记节头

  ```shell
  objdump -h 2.3_main.o
  ```

  列表名: `VMA`(Virtual Memory Address),`LMA`(Load Memory Address)

  `call` 指令注意点：此时要调用`func`函数，call后的4个字节是 callee 相对于 **调用指令的下一条指令** 的偏移量

  ```assembly
    401d06:       e8 07 00 00 00          call   401d12 <func>
    401d0b:       b8 00 00 00 00          mov    eax,0x0
    401d10:       c9                      leave
    401d11:       c3                      ret  
    0000000000401d12 <func>:
    401d12:       f3 0f 1e fa             endbr64
    401d16:       55                      push   rbp 
  ```

  `0xe8`是指令`call`的机器码，注意到其后（注意四个字节都要算）的`0x00000007`,此时`PC=0x401d0b`(原下一条指令位置)由`0x401d0b + 0x7 = 0x401d12`正好是`func`的入口代码地址

##### 静态链接库

* 后缀名`.a`为静态链接库文件，如`libc.a`

* 可视为一组目标文件经过压缩打包后形成的集合

* 为了方便管理，使用`ar`指令将目标文件进行压缩、编号、索引，形成`*.a`文件

* 如printf.o, scanf.o, malloc.o ... 由

  ```shell
  ar -t libc.a
  ```

  生成`libc.a`

## 动态链接

* 动态链接：运行或加载时在内存中完成链接的过程

* 用于动态链接的系统库为**共享库**，或**共享对象**

* 操作

  > 将`func.c`编译为共享库，并用该库编译`main.c`

  生成共享库`func.so`

  ```shell
  gcc -shared -fpic -o func.so func.c
  ```

  * `-fpic` ：作用于编译阶段，告诉编译器产生与位置无关代码(Position-Independent Code)，则产生的代码中，没有绝对地址，**全部使用相对地址**，故而代码可以被加载器加载到内存的任意位置，都可以正确的执行。这正是**共享库**所要求的，共享库被加载时，在内存的位置不是固定的。
  * `-shared` : 表示生成共享库

  生成动态链接文件`func.ELF2`(且不使用**堆栈保护机制**)

  ```shell
  gcc -fno-stack-protector -o func.ELF2 main.c ./func.so
  ```

  查看使用的共享库

  ```shell
  ldd func.ELF2
  # 段注释
  <<'COMMENT'
  ***** result *****
  linux-vdso.so.1 (0x00007ffc64f66000)                                                             ./func.so (0x00007f83b8a60000)         <--func.so                                              libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f83b8867000)                               /lib64/ld-linux-x86-64.so.2 (0x00007f83b8a6c000)   <--动态加载器(本身也是共享库)
  ******************
  COMMENT
  ```

  * `ldd` : 打印程序或库文件所依赖的**共享库**列表

* 位置无关代码`PIC`
  * `PIC(Position-Independent Code)` : 可以加载且无需重定位的代码
  * `GOT(Global Offset Table)` : 全局偏移量表(程序中`.data`和`.text`的相对距离总保持不变，因此相对地址是运行时常量)，GOT位于`Data Segment`的开头，用于保存**全局变量**和**库函数**的引用，每条目占8-bytes，加载时会进行重定位并填入符号的绝对地址
  * `GOT`内容分块
    * `.got` : 保存全局变量引用
    * `.got.plt` : 保存函数引用

* 延迟绑定`lazy binding`

  * 基本思想：当函数第一次被调用动态链接器才进行符号查找，重定位等操作，未被调用则不绑定
  * 实现规则：`ELF`文件通过`PLT(Procedure Linkage Table, 过程链接表)`和`GOT`配合实现延迟绑定，每个被调用的库函数都有一组对应的PLT和GOT

* 运行时链接

  * 程序在运行时加载和链接共享库

  * linux 为此提供了API `dlopen`, 传统动态链接会生成一个`GOT`表，记录着所有可能用到的符号.

    ```C
    #include <dlfcn.h>
    void *dlopen(const char* filename, int flags);
    int dlclose(void *handle)
    ```

    





