;var4 = (var1 * -5) / (-var2 % var3)
;var1 @ 0x600000-600004
;var2 @ 0x600004-600008
;var3 @ 0x600008-60000c
;var4 @ 0x60000c-600010

mov ebx, DWORD PTR [0x600000]
imul ebx, -5

mov edx, 0
mov eax, DWORD PTR [0x600004]
neg eax
mov ecx, DWORD PTR [0x600008]
div ecx

mov eax, ebx
mov ebx, edx
mov edx, 0
div ebx
mov DWORD PTR [0x60000c], eax
done:
