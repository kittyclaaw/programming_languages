%define SYSTEM_READ 0
%define SYSTEM_WRITE 1
%define SYSTEM_EXIT 60
%define STDOUT 1
%define STDIN 0
%define STDERR 2


global exit
global string_length
global print_string
global print_newline
global print_char
global print_int
global print_uint
global string_equals
global read_char
global read_word
global parse_uint
global parse_int
global string_copy



section .text

; Принимает код возврата и завершает текущий процесс
exit:
        mov     rax, SYSTEM_EXIT
        syscall


; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
        xor     rax, rax
.loop:
        cmp     byte[rax+rdi], 0
        je      .end
        inc     rax
        jmp     .loop
.end:
   ret


; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
        push    rdi
        call    string_length
        pop     rsi
        mov     rdx, rax
        mov     rax, SYSTEM_WRITE
        mov     rdi, STDOUT
        syscall
   ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
        mov     rdi, 0xA


; Принимает код символа и выводит его в stdout
print_char:
        push    rdi
        mov     rax, SYSTEM_WRITE
        mov     rsi, rsp
        mov     rdi, STDOUT
        mov     rdx, 1
        syscall
        pop     rdi
   ret


; Выводит знаковое 8-байтовое число в десятичном формате
print_int:
        test    rdi, rdi
        jns     print_uint
        push    rdi
        mov     rdi, '-'
        call    print_char
        pop     rdi
        neg     rdi


; Выводит беззнаковое 8-байтовое число в десятичном формате
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
        mov     r9, 10
        ;mov     r8, 1
        mov     rax, rdi
        mov     rcx, rsp
        sub     rsp, 32
        dec     rcx
        mov     byte[rcx], 0
.loop:
        xor     rdx, rdx
        ;inc     r8
        div     r9
        add     rdx, '0'
        dec     rcx
        mov     byte[rcx], dl
        test    rax, rax
        jne     .loop
.print:
        mov     rdi, rcx
        ;push    r8
        call    print_string
        ;pop     r8
        add     rsp, 32
    ret


; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
        xor     rax, rax
.loop:
        mov     bl, [rdi+rax]
        cmp     bl, [rax+rsi]
        jne     .not
        test    bl, bl
        je      .ok
        inc     rax
        jmp     .loop
.ok:
        mov     rax, 1
   ret
.not:
        xor     rax, rax
   ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
        push    0
        mov     rax, SYSTEM_READ
        mov     rdi, STDIN
        mov     rsi, rsp
        mov     rdx, 1
        syscall
        pop rax
     ret


; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор
read_word:
        push    r12
        push    r13
        push    r14
        mov     r12, rdi
        mov     r13, rsi
        xor     r14, r14
.loop:        
        call    read_char
        cmp     rax, 0x20
        je      .skip
        cmp     rax, 0x9
        je      .skip
        cmp     rax, 0xA
        je      .skip
        cmp     r13, r14
        jng     .err
        test    rax, rax
        je      .good
        mov     [r12+r14], al
        inc     r14
        jmp     .loop
.skip:
        test    r14, r14
        je      .loop
.good:
        mov     byte[r12+r14], 0
        mov     rax, r12
        mov     rdx, r14
        jmp     .end
.err:
        xor     rax, rax
.end:
        pop     r14
        pop     r13
        pop     r12
     ret


; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
        xor     rax, rax
        mov     r9, 10        
        xor     r8, r8
        xor     rdx, rdx
.loop:
        mov     r8b, byte[rdi+rdx]
        cmp     r8b, '0'
        jl      .end
        cmp     r8b, '9'
        jg      .end
        push    rdx
        mul     r9
        pop     rdx
        sub     r8, '0'
        add     rax, r8
        inc     rdx
        jmp     .loop
.end:
    ret


; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был)
; rdx = 0 если число прочитать не удалось
parse_int:
        xor     rax, rax
        cmp     byte[rdi], '-'
        je      .neg
        cmp     byte[rdi], '+'
        je      .pos
        jmp     parse_uint
.pos:
        inc     rdi
        push    rdi
        call    parse_uint
        pop     rdi
        inc     rdx
     ret
.neg:
        inc     rdi
        push    rdi
        call    parse_uint
        pop     rdi
        neg     rax
        inc     rdx
     ret

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
string_copy:
        xor     rax, rax
.loop:
        cmp     rax, rdx
        je      .err
        mov     r8b, [rdi+rax]
        mov     [rsi+rax], r8b
        inc     rax
        test    r8b, r8b
        je      .end
        jmp     .loop
.err:
        xor     rax, rax
.end:
   ret

