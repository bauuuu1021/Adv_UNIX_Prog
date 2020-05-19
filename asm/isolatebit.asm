mov bx, WORD PTR [0x600001]
shr ax, 5
and ax, 127
mov WORD PTR [0x600000], ax
mov WORD PTR [0x600001], bx
done:  
