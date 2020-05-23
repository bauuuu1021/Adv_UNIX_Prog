;let toIndex = arr.length
;  while (toIndex > 1) {
;    toIndex--
;    for (let i = 0; i < toIndex; i++) {
;      // 如果前面的元素比後面的元素要大，則交換元素位置
;      if (arr[i] > arr[i + 1]) {
;        let tempValue = arr[i]
;        arr[i] = arr[i + 1]
;        arr[i + 1] = tempValue
;      }
;    }
;  }
;  return arr
;}

	mov ecx, 10
WHILE:	cmp ecx, 1
	jle END
	dec ecx

	mov edx, 0
FOR:	cmp edx, ecx
	jge WHILE
	
	mov eax, DWORD PTR [0x600000+edx*4]
	mov ebx, DWORD PTR [0x600004+edx*4]

	cmp eax, ebx
	jle CONT
	mov DWORD PTR [0x600000+edx*4], ebx
	mov DWORD PTR [0x600004+edx*4], eax

CONT:	inc edx
	jmp FOR

END:
done:
