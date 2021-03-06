DEFINE_MACROS=-DCONFIG_X86_32 -DCONFIG_PREEMPT -DCONFIG_MMU

export CFLAGS=-g -Wall -mregparm=3 -march=i386 -m32 -mpreferred-stack-boundary=2 -fomit-frame-pointer -ffreestanding -fno-toplevel-reorder -fno-strict-aliasing -fno-stack-protector -nostdinc -I../include -D__KERNEL__ $(DEFINE_MACROS)

export AFLAGS:=$(CFLAGS) -D__ASSEMBLY__

SUB_OBJS=kernel/kernel.o mm/mm.o lib/lib.o

ALL:setup/setup.bin kernel.bin

setup/setup.bin:
	cd setup; make clean; make;

kernel.bin:kernel.elf
	objcopy -O binary kernel.elf kernel.bin -S -R .comment -R .eh_frame 

kernel.elf:$(SUB_OBJS)
	ld -Tkernel.ld -nostdlib -M -o kernel.elf $(SUB_OBJS) > kernel.map

kernel/kernel.o:
	cd kernel; make clean; make;

fs/fs.o:
	cd fs; make clean; make;

mm/mm.o:
	cd mm; make clean; make;

lib/lib.o:
	cd lib; make clean; make;

clean:
	cd setup; make clean;
	cd kernel; make clean;
	cd fs; make clean;
	cd mm; make clean;
	cd lib; make clean;
	-rm -v kernel.bin kernel.elf kernel.map
