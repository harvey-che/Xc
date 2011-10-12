OBJ_S=head_32.o
OBJ_C=init.o
OBJ_FILES:=$(OBJ_S) $(OBJ_C)

kerenl.bin:kernel.elf
	objcopy -O binary kernel.elf kernel.bin -R .comment -S
#	objcopy -O binary kernel.elf kernel.bin -j .text -j .rodata -j .data -j .bss

kernel.elf:$(OBJ_FILES)
	ld -Tkernel.ld -M -o kernel.elf $(OBJ_FILES)>kernel.map

CFLAGS=-g -mregparm=3 -nostdinc -nostdlib -ffreestanding -fno-toplevel-reorder -fno-strict-aliasing -fno-stack-protector -Iinclude -I../include -D__KERNEL__

AFLAGS=$(CFLAGS) -D__ASSEMBLY__

$(OBJ_S):%.o:%.S
	gcc $(AFLAGS) -o $@ -c $<

$(OBJ_C):%.o:%.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	-rm -v $(OBJ_FILES) kernel.elf kernel.bin kernel.map
