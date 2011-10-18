SEG_BOOTSECT=0x07c0
SEG_BOOTSECT2=0x9000
SEG_SETUP=0x9000
OFFSET_SETUP=0x200
SEG_KERNEL=0x1000

/* size of the sections in sector, so the setup is 2k in byte */
SIZE_SETUP=17
/* the kernel is 512K in byte */
SIZE_KERNEL=1024


.code16gcc
.section ".bstext", "ax"
.global _start
_start:
    /* Reset %cs */
    ljmp $SEG_BOOTSECT, $_start2 
_start2:

    movw $SEG_BOOTSECT, %ax
    movw %ax, %ds
    movw $SEG_BOOTSECT2, %ax
	movw %ax, %es
	movw $256, %cx
	xorw %si, %si
	xorw %di, %di
	cld
	rep; movsw
	ljmp $SEG_BOOTSECT2, $_start3

_start3:
    /* Initialize segment registers */
	cli
	movw %ax, %ds
	movw %ax, %ss
	movw $0xfffc, %sp
    xorw %bp, %bp
    
    /* Clear the screen and set the position of the cursor */
	movw $0x0600, %ax
	movw $0x0700, %bx
	xorw %cx, %cx
	movw $0x184f, %dx
	int $0x10

	xorw %bx, %bx
	xorw %dx, %dx
	movw $0x0200, %ax
	int $0x10

    /* Get disk drive parameters */
    movw    $0x0800, %ax
	xorb    %dl, %dl
	int     $0x13

    incb %ch
	movb %ch, max_cylinders 
	xorb %ch, %ch  
    andb $0x3F, %cl              /* ignore the most 2 significant bits */
	incb %cl
	movb %cl, sectors_per_track
	incb %dh
	movb %dh, max_heads
    

	/* Load the setup code to SEG_SETUP */
	pushw $20
	pushw $SEG_BOOTSECT2
	pushw $str_loading_start
	call print_str

	pushw $SEG_SETUP
	pushw $OFFSET_SETUP
	pushw $SIZE_SETUP
	pushw $0
	pushw $0
	pushw $2
	call load_sectors 
    cmpw $0, %ax
	jne 1f

    /* Load the Xc kernel to SEG_KERNEL */
	pushw $0x1000
	pushw $0x0000
	pushw $SIZE_KERNEL
	pushw $0
	pushw $1
	pushw $1  /* the CHS of the kernel head is related to the setup size */
    call load_sectors

	cmpw $0, %ax
	jne 1f

	pushw $15
	pushw $SEG_BOOTSECT2
	pushw $str_loading_done
	call print_str

    /* Zero the bss, %es was modified by previous int call */
	movw $SEG_BOOTSECT2, %ax
	movw %ax, %es
	movw $__bss_start, %di
	movw $_end+3, %cx
	xorl %eax, %eax
	subw %di, %cx
	shrw $2, %cx
	rep; stosl
    
	call prepare_for_pm
1:
    pushw $22
	pushw $SEG_BOOTSECT2
	pushw $str_loading_failed
	call print_str
die: jmp die
/*
 word0:word1:word2 the sector number, head and cylinder of the starting sector
 word3 the nubmer of sectors to be read
 word5:word4 the address to store the data
 if OK return 0, else return -1
*/
load_sectors:  
    pushw %bp
	movw %sp, %bp
	subw $2, %sp
	pushaw
	pushw %es

    /*
	# 16(%bp)    segment
	# 14(%bp)    offset
	# 12(%bp)    rest sectors
	# 10(%bp)    current cylinder
	# 8(%bp)     current head
	# 6(%bp)     current sector
	*/
    movw $0, -2(%bp) 
loading_continue:
	movw sectors_per_track, %ax
	subw 6(%bp), %ax
    cmpw 12(%bp), %ax
	jb 1f
    movw 12(%bp), %ax
1:
    movw $128, %bx
    cmpw $0, 14(%bp)
	je 1f
	xorw %cx,%cx
	subw 14(%bp), %cx
	shrw $9, %cx
	movw %cx, %bx
1:
	cmpw %bx, %ax
	jb 1f
	movw %bx, %ax
1:                      /* how many sectors to be read is ready in %al */
    
    movb 6(%bp), %cl
	movb 8(%bp), %dh
	movb 10(%bp), %ch
	les 14(%bp), %bx
	movb $0x02, %ah
	xorb %dl, %dl
	int $0x13
	jc ps_err_end
    
    addb %al, %cl
	cmpb sectors_per_track, %cl
	jb 1f
	movb $1, %cl

	incb %dh
1:
    movb %cl, 6(%bp)         /* the sector to start reading in next iteration is updated */
	cmpb max_heads, %dh
	jb 1f
	xorb %dh, %dh
	incb %ch
1:
    movb %dh, 8(%bp)          /* the head to start reading in next iteration is updated */
    movb %ch, 10(%bp)         /* the cylinder to start reading in next iteration is updated */


    xorb %ah, %ah               /* %ax is the sectors this iteration have loaded */
	subw %ax, 12(%bp)           /* the rest sectors to be loaded is updated */
	shlw $9, %ax
	addw %ax, %bx
	movw %es, %ax
    
	jnc 1f            /* jump if %bx overflows */
	addw $0x1000, %ax
	xorw %bx, %bx
1:
    movw %bx, 14(%bp)
	movw %ax, 16(%bp)

	cmpw $0, 12(%bp)
	jne loading_continue
    jmp ps_ok_end

ps_err_end:
    movw $0xFFFF, -2(%bp)
ps_ok_end:
    popw %es
	popaw
	popw %ax
	popw %bp
    ret $12
/* end load_sectors */

/*
word1:word0 segment:offset of the string
word2       the length of the string
*/
print_str:
    pushw %bp
	movw %sp, %bp
    pushaw
	pushw %es
    
	/*
	# 10(%bp)    length
	# 8(%bp)   segment
	# 6(%bp)   offset
    */

    movb $3, %ah
	xorb %bh, %bh
	int $0x10           /* get current cursor position in %dx */

    movw 10(%bp), %cx
    movw $0x1301, %ax
	movw $0x0007, %bx
    pushw %bp
	les 6(%bp), %bp
    int $0x10
    popw %bp

    popw %es
    popaw
	popw %bp
    ret $6
/* end print_str */

str_loading_start:
    .ascii "Loading kernel ...\r\n"  #20
str_loading_done:
    .ascii "Loading done!\r\n"        #15
str_loading_failed:
    .ascii "Sad, loading failed.\r\n"   #22
sectors_per_track:
    .word 0
max_cylinders:
    .word 0
max_heads:
    .word 0
	.org 510
    .word 0xAA55




