	mov ebx, 15
L:	sub ebx, 1
	mov al, BYTE PTR [0x600000+ebx]
	or al, 0x20
	mov BYTE PTR [0x600010+ebx], al
	test ebx, ebx
	jnz L
done:
