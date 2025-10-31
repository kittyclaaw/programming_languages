BITS 64
default rel

section .rodata
    align 16
    m_row_1: dd 0.272, 0.349, 0.393, 0.272
    m_row_2: dd 0.534, 0.686, 0.769, 0.534
    m_row_3: dd 0.131, 0.168, 0.189, 0.131

section .text
global sepia_asm_helper

;-------------------------------------------------------------------------
; Макрос обработки группы из 6 байт (2 пикселя):
; Аргумент %1 = номер группы (0, 1 или 2).
;-------------------------------------------------------------------------
%macro do_group 1
    pxor xmm0, xmm0
    pxor xmm1, xmm1
    pxor xmm2, xmm2

    ;; Шесть байт для двух пикселей (B, G, R, B, G, R)
    mov rax, [rdi + %1*6]

    ;; -- Первый пиксель: B0, G0, R0
    mov cl, al             ; B0
    pinsrb xmm0, cl, 0
    shr rax, 8

    mov cl, al             ; G0
    pinsrb xmm1, cl, 0
    shr rax, 8

    mov cl, al             ; R0
    pinsrb xmm2, cl, 0
    shr rax, 8

    ;; -- Второй пиксель: B1, G1, R1
    mov cl, al             ; B1
    pinsrb xmm0, cl, 4
    shr rax, 8

    mov cl, al             ; G1
    pinsrb xmm1, cl, 4
    shr rax, 8

    mov cl, al             ; R1
    pinsrb xmm2, cl, 4
    shr rax, 8

    ;; Конвертируем int32 → float
    cvtdq2ps xmm0, xmm0
    cvtdq2ps xmm1, xmm1
    cvtdq2ps xmm2, xmm2

    ;; Выбираем pattern shufps в зависимости от группы
    %if %1 = 0
        ;; p1, p1, p1, p2
        %define pattern 0b01000000
    %elif %1 = 1
        ;; p2, p2, p3, p3
        %define pattern 0b01010000
    %elif %1 = 2
        ;; p3, p4, p4, p4
        %define pattern 0b01010100
    %endif

    shufps xmm0, xmm0, pattern
    shufps xmm1, xmm1, pattern
    shufps xmm2, xmm2, pattern

    ;; Матричные умножения
    mulps xmm0, xmm3
    mulps xmm1, xmm4
    mulps xmm2, xmm5

    addps xmm0, xmm1
    addps xmm0, xmm2

    ;; Переводим float → int32 + saturate до [0..255]
    cvtps2dq xmm0, xmm0
    packusdw xmm0, xmm0
    packuswb xmm0, xmm0

    ;; Записываем результат (4 байта) одним movd
    movd [rsi + %1*4], xmm0

    ;; прокручиваем матрицу на следующий пиксель
    shufps xmm3, xmm3, 0b01001001
    shufps xmm4, xmm4, 0b01001001
    shufps xmm5, xmm5, 0b01001001

%endmacro

;-------------------------------------------------------------------------
; Функция sepia_asm_helper
; rdi — адрес входного буфера (хранит RGB/BGR-данные),
; rsi — адрес выходного буфера,
; мы обрабатываем 3 группы (всего 6 байт * 3 = 18 байт входа),
; на выходе получаем 3 * 4 байта = 12 байт (по 4 на группу).
;-------------------------------------------------------------------------
sepia_asm_helper:
    push rbp

    ;; Подгружаем начальную матрицу
    ;; Группа 0: b, g, r, b
    ;; Группа 1: g, r, b, g
    ;; Группа 2: r, b, g, r
    movaps xmm3, [m_row_1]
    movaps xmm4, [m_row_2]
    movaps xmm5, [m_row_3]

    ;; Обрабатываем 3 группы подряд
    do_group 0
    do_group 1
    do_group 2

    pop rbp
    ret
