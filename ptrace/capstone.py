from pwn import *
import binascii as b

r = remote('aup.zoolab.org', 2530)
context.arch = 'x86_64'

for i in range(10):
    r.recvuntil(b'========================================================\n')
    ques = r.recvline().decode()
    l = ques.split()
    s = l[1]
    d = disasm(b.a2b_hex(s), byte=0, offset=0)
    d = d+'\n'
    
    r.recv()
    print(b.b2a_hex(d.encode()))
    r.sendline(b.b2a_hex(d.encode()))
    print(r.recvline())

print(r.recvline())
