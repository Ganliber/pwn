# 关于 brop 题目

## 环境部署

1. 首先安装`socat`

   ```python
   sudo apt install socat
   ```

2. 开两个`terminal`，一个模拟服务器端，执行

   ```python
   socat tcp-l:9999,fork exec:./brop,reuseaddr
   ```

3. 另一端模拟`hacker`，进行脚本攻击（盲打）

4. 服务器端`ldd`查看`libc`库和链接器`ld`版本

   ```python
   ldd brop
   	linux-vdso.so.1 (0x00007ffff7fcd000)
   	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007ffff7dc1000)
   	/lib64/ld-linux-x86-64.so.2 (0x00007ffff7fcf000)
   ```

## 脚本攻击

```python
exp.py : 获取 buffer_length, stop_gadget, brop_gadget, rdi_ret, puts@plt,
exp_puts_got.py: 获取内存泄漏二进制文件 code, 进而得到 puts@got
exp_final.py: 获取libc版本, libcBase, system_addr, str_bin_sh_addr, ret(=rdi_ret+0x1) 并实现最终攻击
```





