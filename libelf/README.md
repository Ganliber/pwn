# ELF and Libelf

> **Task：**
>
> 负责 core dump 文件格式，如何插入 tracing log，如何从中解析（了解 elf 文件格式）
>
> **Note：**
>
> This is about the use of libelf

## Starting example

***

>   第一个参数，int型的argc，为整型，用来统计程序运行时发送给main函数的命令行参数的个数，在VS中默认值为1。
>   第二个参数，char**型的argv[]，为字符串数组，用来存放指向的字符串参数的指针数组，每一个元素指向一个参数。各成员含义如下：
>         argv[0]指向程序运行的全路径名
>         argv[1]指向在DOS命令行中执行程序名后的第一个字符串
>         argv[2]指向执行程序名后的第二个字符串
>         argv[3]指向执行程序名后的第三个字符串
>         argv[argc]为NULL

对于argc，有如下实例

```shell
# for program execA, the argc includes your executable file
./execA  # argc=1
./execA a.txt  # argc=2
./execA a.txt b.txt  # argc=3
```

示例代码

```C
/*
* Getting started with libelf .
*
* $Id : prog1 . txt 2133 2011 -11 -10 08:28:22 Z jkoshy $
*/
# include < err .h >
# include < fcntl .h >
# include < libelf .h >
# include < stdio .h >
# include < stdlib .h >
# include < unistd .h >
int main ( int argc , char ** argv )
{
    int fd ;
    Elf * e ;//a handle to ELF object
    char * k ;
    Elf_Kind ek ; 

    /**/
    if( argc != 2)
        errx ( EXIT_FAILURE , " usage: %s file-name " , argv[0]);
    
    /*libelf version not matched*/
    if( elf_version ( EV_CURRENT ) == EV_NONE )
        errx ( EXIT_FAILURE , " ELF library initialization "
                " failed : %s " , elf_errmsg(-1));

    /**/
    if(( fd = open ( argv[1] , O_RDONLY , 0)) < 0)
        err ( EXIT_FAILURE , " open \%s \" failed " , argv[1]);

    /**/
    if(( e = elf_begin( fd , ELF_C_READ 5, NULL )) == NULL )
        errx ( EXIT_FAILURE , " elf_begin () failed : %s . " ,
            elf_errmsg ( -1)); 

    ek = elf_kind( e );

    switch ( ek ) {
        case ELF_K_AR :
            k = " ar (1) archive " ;
            break ;
        case ELF_K_ELF :
            k = " elf object " ;
            break ;
        case ELF_K_NONE :
            k = " data " ;
            break ;
        default :
            k = " unrecognized " ;
    }
    ( void ) printf( " %s : %s \ n " , argv[1] , k );
    ( void ) elf_end(e);
    ( void ) close(fd);
    exit( EXIT_SUCCESS );
}
```





## ELF

***

> Executable and Linkable Format

* `version 1.2` : 32-bit Intel Architecture operating environments.

* `origin` : developed and published by UNIX System Laboratories (`USL`) as part of the Application Binary Interface (`ABI`).







## libelf

***

> Repo : `cpredump_parser`
>
> * include : `elf_parser_base.h`
> * src : `elf_`







































