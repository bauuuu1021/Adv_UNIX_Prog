mov eax, DWORD PTR [0x600000]
add eax, DWORD PTR [0x600004]
imul eax, DWORD PTR [0x600008]
mov DWORD PTR [0x60000c], eax
done:
