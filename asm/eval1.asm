mov eax, DWORD PTR [0x600004]
mov ebx, DWORD PTR [0x600008]
sub eax, ebx
mov ecx, DWORD PTR [0x600000]
sub eax, ecx
mov DWORD PTR [0x60000c], eax
done:
