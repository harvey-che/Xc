OBJ_S=bootsect.o bioscall.o pmjump.o copy.o
OBJ_C=main.o memory.o pm.o regs.o tty.o a20.o 
OBJ_FILES:=$(OBJ_S) $(OBJ_C)

setup.bin:setup.elf
	objcopy -O binary setup.elf setup.bin -S -R .comment 
#	objcopy -O binary setup.elf setup.bin -j .bstext -j .bsdata -j .inittext -j .initdata -j .text -j .text32 -j .rodata -j .data -j .bss

setup.elf:$(OBJ_FILES)
	ld -Tsetup.ld -nostdlib -M -o setup.elf $(OBJ_FILES)>setup.map

#CFLAGS=-g -mregparm=3 -nostdinc -ffreestanding -fno-toplevel-reorder -fno-strict-aliasing -fno-stack-protector -Iinclude -I../include -D__KERNEL__

CFLAGS=-g -mregparm=3 -nostdinc  -fno-strict-aliasing -include code16gcc.h -Iinclude -I../include -D__KERNEL__
AFLAGS=$(CFLAGS) -D__ASSEMBLY__

$(OBJ_S):%.o:%.S
	gcc $(AFLAGS) -o $@ -c $<

$(OBJ_C):%.o:%.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	-rm -v $(OBJ_FILES) setup.elf setup.bin setup.map
