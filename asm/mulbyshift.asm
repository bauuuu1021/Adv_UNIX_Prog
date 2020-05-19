mov eax, DWORD PTR [0x600000]
mov ebx, DWORD PTR [0x600000]
shl eax, 5
shl ebx, 1
sub eax, ebx
shl ebx, 1
sub eax, ebx
mov DWORD PTR [0x600004], eax
done:
