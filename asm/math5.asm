;var3 = (var1 * -var2) / (var3 - ebx)
;	note: overflowed part should be truncated
;======
;var1 @ 0x600000-600004
;var2 @ 0x600004-600008
;var3 @ 0x600008-60000c

mov ecx, DWORD PTR [0x600004]
neg ecx
mov eax, DWORD PTR [0x600000]
mul ecx
and eax, 0xffffffff
mov edx, 0

mov ecx, DWORD PTR [0x600008]
sub ecx, ebx

div ecx
mov DWORD PTR [0x600008], eax
done:
