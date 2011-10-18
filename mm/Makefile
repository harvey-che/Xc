OBJ_S:=
OBJ_C:=init_32.o
OBJ_FILES:=$(OBJ_S) $(OBJ_C)


mm.o:$(OBJ_FILES)
	ld -r -M -o mm.o $(OBJ_FILES)>mm.map


CFLAGS=-g -Wall -mregparm=3 -march=i386 -m32 -mpreferred-stack-boundary=2 -fomit-frame-pointer -ffreestanding -fno-toplevel-reorder -fno-strict-aliasing -fno-stack-protector -nostdinc -I../include -D__KERNEL__

AFLAGS:=$(CFLAGS) -D__ASSEMBLY__

$(OBJ_S):%.o:%.S
	gcc $(AFLAGS) -o $@ -c $<

$(OBJ_C):%.o:%.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	-rm -v $(OBJ_FILES) mm.o mm.map
