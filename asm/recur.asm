;r(n) = 0, if n <= 0
;        = 1, if n == 1
;        = 2*r(n-1) + 3*r(n-2), otherwise

	mov rax, 15
	call REC
	pop rax
	jmp E

REC:	pop rbx

	cmp rax, 0
	jg NON_0
	mov rax, 0
	push rax
	push rbx
	ret

NON_0:	cmp rax, 1
	jne NON_1
	mov rax, 1
	push rax
	push rbx
	ret

NON_1:	
	; 2*r(n-1)
	push rbx	; origin ret address
	push rax	; origin input number
	
	dec rax
	call REC
	pop rcx
	add rcx, rcx
	
	pop rax		; origin ret address
	pop rbx		; origin input number
	
	; 3*r(n-2)
	push rbx	; origin ret address
	push rax	; origin input number
	push rcx
	
	sub rax, 2
	call REC
	pop rdx
	imul rdx, 3

	pop rcx	
	pop rax		; origin ret address
	pop rbx		; origin input number

	mov rax, rcx
	add rax, rdx
	push rax	; ret value
	push rbx	; ret address
	ret	
E:
done:
