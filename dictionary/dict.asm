%include "lib.inc"
global find_word

%define OFFSET 8  ; Вынесенный magic number

find_word:
    push    r12
.loop:
    mov     r12, rsi
    lea     rsi, [rsi+OFFSET]

	call	string_equals
	test	rax, rax
	jz	.next
	mov	rax, r12
	pop	r12
	ret
.next:
	mov	rsi, [r12]
	test	rsi, rsi
	jnz	.loop
	xor	rax, rax
	pop	r12
	ret
