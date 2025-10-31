section .text

%define READ 0
%define WRITE 1
%define EXIT 60
%define STDIN 0
%define STDOUT 1
%define DIVIDER 10

exit:
    mov rax, EXIT
    syscall

string_length:
    xor rax, rax
.loop:
    cmp byte [rdi + rax], 0
    jz .break
    inc rax
    jmp .loop
.break:
    ret

print_string:
    push rdi
    call string_length
    pop rsi
    mov rdx, rax
    mov rax, WRITE
    mov rdi, STDOUT
    syscall
    ret

print_char:
    push rdi
    mov rsi, rsp
    mov rdx, 1
    mov rax, WRITE
    mov rdi, STDOUT
    syscall
    pop rdi
    ret

print_newline:
    mov rdi, 0xA
    call print_char
    ret

print_uint:
    mov rax, rdi
    mov r8, DIVIDER
    mov rcx, rsp
    sub rsp, 24
    dec rcx
    mov byte [rcx], 0
.loop:
    xor rdx, rdx
    div r8
    add dl, '0'
    dec rcx
    mov byte [rcx], dl
    test rax, rax
    jnz .loop
    mov rdi, rcx
    call print_string
    add rsp, 24
    ret

print_int:
    test rdi, rdi
    jns print_uint
    push rdi
    mov rdi, '-'
    call print_char
    pop rdi
    neg rdi
    jmp print_uint

string_equals:
    xor rcx, rcx
.loop:
    mov al, byte [rsi + rcx]
    cmp al, [rdi + rcx]
    jne .bad
    inc rcx
    test al, al
    jnz .loop
.good:
    mov rax, 1
    ret
.bad:
    xor rax, rax
    ret

read_char:
    xor rax, rax
    sub rsp, 8
    mov rdi, STDIN
    mov rsi, rsp
    mov rdx, 1
    syscall
    test rax, rax
    jle .return
    mov al, [rsp]
.return:
    add rsp, 8
    ret

read_word:
    push rbx
    push r12
    push r13
    xor rbx, rbx
    mov r12, rsi
    mov r13, rdi
.all_spaces:
    call read_char
    cmp al, 0x20
    je .all_spaces
    cmp al, 0xA
    je .all_spaces
    cmp al, 0x9
    je .all_spaces
    test al, al
    jz .bad
.skip_test:
    cmp rbx, r12
    je .bad
    mov byte [r13 + rbx], al
    inc rbx
    call read_char
    cmp al, 0x20
    je .good
    cmp al, 0xA
    je .good
    cmp al, 0x9
    je .good
    test al, al
    jne .skip_test
.good:
    mov byte [r13 + rbx], 0
    mov rax, r13
    mov rdx, rbx
    pop r13
    pop r12
    pop rbx
    ret
.bad:
    xor rax, rax
    xor rdx, rdx
    pop r13
    pop r12
    pop rbx
    ret

parse_uint:
    xor rdx, rdx
    xor rax, rax
    xor rcx, rcx
.read_next:
    mov al, [rdi + rcx]
    test al, al
    jz .exit
    sub al, '0'
    js .exit
    cmp al, 9
    ja .exit
    imul rdx, 10
    add rdx, rax
    inc rcx
    jmp .read_next
.exit:
    mov rax, rdx
    mov rdx, rcx
    ret

parse_int:
    mov al, [rdi]
    cmp al, '-'
    je .neg_signed
    cmp al, '+'
    jne parse_uint
.neg_signed:
    push rdi
    inc rdi
    call parse_uint
    pop rdi
    cmp byte [rdi], '+'
    je .skip
    neg rax
.skip:
    test rdx, rdx
    je .exit
    inc rdx
.exit:
    ret

string_copy:
    push rbx
    push r12
    push r13
    mov rbx, rdi
    mov r12, rsi
    mov r13, rdx
    call string_length
    cmp r13, rax
    jl .bad
    mov rcx, rax
    inc rcx
    mov rsi, rbx
    mov rdi, r12
    rep movsb
    jmp .exit
.bad:
    xor rax, rax
.exit:
    pop r13
    pop r12
    pop rbx
    ret
