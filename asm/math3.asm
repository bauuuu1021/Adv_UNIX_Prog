mov eax, DWORD PTR [0x600000]
imul eax, 5
mov ebx, DWORD PTR [0x600004]
sub ebx, 3
div ebx
mov DWORD PTR [0x600008], eax
done:
