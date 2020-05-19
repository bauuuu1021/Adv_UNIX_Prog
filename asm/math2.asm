mov eax, DWORD PTR [0x600000]
imul eax, -1
imul eax, DWORD PTR [0x600004]
add eax, DWORD PTR [0x600008]
done:
