#!/bin/bash

# Удаляем предыдущие результаты
rm -f tester src/sepia_filter.o

# Компилируем ассемблерный файл в объектный файл
nasm -f elf64 -o src/sepia_filter.o src/sepia_filter.asm
if [ $? -ne 0 ]; then
    echo "Failed to assemble src/sepia_filter.asm"
    exit 1
fi

# Компилируем программу
gcc -o tester sepia_tester.c src/transform.c src/io_bmp.c src/image.c src/sepia_filter.o -Iinclude -Wall -Werror -O2 -std=c17
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Запускаем тестер
if [ -z "$1" ]; then
    echo "Usage: ./run_tester.sh <input BMP file>"
    exit 1
fi

./tester "$1"
