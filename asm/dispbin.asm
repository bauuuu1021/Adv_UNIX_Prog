;for example, if AX = 0x1234, the result should be:
;str1 = 0001001000111000
;str1 @ 0x600000-600014

	mov ecx, 16
L_BEG:	dec ecx
	mov edx, 1
	and edx, eax
	
	test edx, edx
	je C	
	mov BYTE PTR [0x600000+ecx], '1'
	jmp L_END
C:	mov BYTE PTR [0x600000+ecx], '0'

L_END:	shr eax, 1
	test ecx, ecx
	jne L_BEG
done:
