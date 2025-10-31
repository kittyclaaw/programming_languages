LD=ld
ASM=nasm
ASMFLAGS=-f elf64
PROGRAM=main
SRC=main.asm lib.asm dict.asm
OBJ=$(SRC:%.asm=%.o)

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

$(PROGRAM): $(OBJ) 
	$(LD) -o $@ $+ 

.PHONY: clean test

test.py: $(PROGRAM)
	python3 test.py

clean:
	rm -f $(PROGRAM) $(OBJ)
