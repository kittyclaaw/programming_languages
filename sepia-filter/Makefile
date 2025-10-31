CFLAGS = -g -O2 -Wall -Werror -std=c17 -Wdiscarded-qualifiers -Wincompatible-pointer-types -Wint-conversion

all: build/io_bmp.o build/image.o build/transform.o build/main.o build/header.o build/sepia_filter.o
	gcc -no-pie -o build/transform $^

build/%.o: src/%.c
	mkdir -p build
	gcc -c $(CFLAGS) -o $@ $<

build/%.o: src/%.asm
	mkdir -p build
	nasm -felf64 -o $@ $<

clean:
	rm -rf build

.PHONY: clean
