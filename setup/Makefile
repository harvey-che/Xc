OBJ_S=bootsect.o bioscall.o pmjump.o copy.o
OBJ_C=main.o memory.o pm.o regs.o tty.o a20.o 
OBJ_FILES:=$(OBJ_S) $(OBJ_C)

setup.bin:setup.elf
	objcopy -O binary setup.elf setup.bin -S -R .comment -R .eh_frame

setup.elf:$(OBJ_FILES)
	ld -Tsetup.ld -nostdlib -o setup.elf $(OBJ_FILES)>setup.map

CFLAGS=-g -Wall -mregparm=3 -march=i386 -m32 -mpreferred-stack-boundary=2 -fomit-frame-pointer -ffreestanding -fno-toplevel-reorder -fno-strict-aliasing -fno-stack-protector -nostdinc -include code16gcc.h -I../include -D__KERNEL__

#CFLAGS=-g -Wall -mregparm=3 -ffreestanding -fno-strict-aliasing -nostdinc -include code16gcc.h -I../include -D__KERNEL__

#CFLAGS=-g -Wall -mregparm=3 -march=i386 -m32 -fno-strict-aliasing -fno-builtin -nostdinc -include code16gcc.h -I../include -D__KERNEL__

AFLAGS=$(CFLAGS) -D__ASSEMBLY__

$(OBJ_S):%.o:%.S
	gcc $(AFLAGS) -o $@ -c $<

$(OBJ_C):%.o:%.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	-rm -v $(OBJ_FILES) setup.elf setup.bin setup.map
