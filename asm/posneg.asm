;posneg: test if registers are positive or negative.
;	if ( eax >= 0 ) { var1 = 1 } else { var1 = -1 }
;	if ( ebx >= 0 ) { var2 = 1 } else { var2 = -1 }
;	if ( ecx >= 0 ) { var3 = 1 } else { var3 = -1 }
;	if ( edx >= 0 ) { var4 = 1 } else { var4 = -1 } 
;      var1 @ 0x600000-600004
;      var2 @ 0x600004-600008
;      var3 @ 0x600008-60000c
;      var4 @ 0x60000c-600010

	cmp eax, 0
	jge A1
	mov DWORD PTR [0x600000], -1
	jmp B
A1:	mov DWORD PTR [0x600000], 1

B:      cmp ebx, 0
        jge B1
        mov DWORD PTR [0x600004], -1
        jmp C
B1:     mov DWORD PTR [0x600004], 1

C:      cmp ecx, 0
        jge C1
        mov DWORD PTR [0x600008], -1
        jmp D
C1:     mov DWORD PTR [0x600008], 1

D:      cmp edx, 0
        jge D1
        mov DWORD PTR [0x60000c], -1
        jmp E
D1:     mov DWORD PTR [0x60000c], 1

E:
done:
