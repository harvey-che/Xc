OBJ_S:=head_32.o
OBJ_C:=init.o printk.o panic.o init_task.o e820.o wait.o sched.o
OBJ_FILES:=$(OBJ_S) $(OBJ_C)

kernel.o:$(OBJ_FILES)
	ld -r -M -o kernel.o $(OBJ_FILES)>kernel.map


CFLAGS=-g -Wall -mregparm=3 -march=i386 -m32 -mpreferred-stack-boundary=2 -fomit-frame-pointer -ffreestanding -fno-toplevel-reorder -fno-strict-aliasing -fno-stack-protector -nostdinc -I../include -D__KERNEL__

AFLAGS=$(CFLAGS) -D__ASSEMBLY__

$(OBJ_S):%.o:%.S
	gcc $(AFLAGS) -o $@ -c $<

$(OBJ_C):%.o:%.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	-rm -v $(OBJ_FILES) kernel.o kernel.map
