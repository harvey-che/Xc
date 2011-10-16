OBJ_S:=
OBJ_C:=
OBJ_FILES:=$(OBJ_S) $(OBJ_C)


mm.o:$(OBJ_FILES)
	ld -r -M -o lib.o $(OBJ_FILES)>lib.map

CFLAGS:=-g -mregparm=3 -nostdinc -ffreestanding -fno-toplevel-reorder -fno-strict-aliasing -fno-stack-protector -I../include -D__KERNEL__

AFLAGS:=$(CFLAGS) -D__ASSEMBLY__

$(OBJ_S):%.o:%.S
	gcc $(AFLAGS) -o $@ -c $<

$(OBJ_C):%.o:%.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	-rm -v $(OBJ_FILES) lib.o lib.map
