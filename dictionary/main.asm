%include "dict.inc"
%include "lib.inc"
%include "words.inc"

%define BUF_SIZE 256
%define SYSTEM_WRITE 1
%define STDERR 2

section .bss
BUF:
resb BUF_SIZE

section .rodata
err_msg1:
db "line reading error", 10, 0
err_msg2:
db "no entry found error", 10, 0

section .text
global _start

_start:
	mov	rdi, BUF
	mov	rsi, 255
	call	read_word
	test	rax, rax
	jnz	.next
.err1:
	mov	rdi, err_msg1
	call	print_error_message
	call	exit
.err2:
	mov	rdi, err_msg2
	call	print_error_message
	call	exit
.next:
	mov	rsi, DICT
	mov	rdi, BUF
	call	find_word
	test	rax, rax
	jz	.err2
	mov	rdi, rax
	add	rdi, 8
	push	rdi
	call	string_length
	pop	rdi
	add	rdi, rax
	inc	rdi
	call	print_string
	call	exit
	
print_error_message:
	push	rdi
	call	string_length
	pop	rsi
	mov	rdx, rax
	mov	rax, SYSTEM_WRITE
	mov	rdi, STDERR
	syscall
	ret
	
