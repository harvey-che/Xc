#include <asm/segment.h>
#include <asm/processor-flags.h>

.code16gcc
.section ".text","ax"
.globl protected_mode_jump

/* %eax entry address of the proetected mode kernel */
protected_mode_jump:
	xorl %ebx, %ebx
	movw %cs, %bx
    shll $4, %ebx
	addl %ebx, 2f
	jmp 1f
1:               /* I wonder if this jump is necessary */

	movl %cr0, %edx
	orb $X86_CR0_PE, %dl
	movl %edx, %cr0

	.byte 0xea
2:	.long in_pm32
	.word __BOOT_CS
/* end protected_mode_jump */
    
	.code32
	.section ".text32", "ax"
.globl in_pm32
in_pm32:
    movw $__BOOT_DS, %cx
	movw $__BOOT_TSS, %di

	movw %cx, %ds
	movw %cx, %es
	movw %cx, %fs
	movw %cx, %gs
	movw %cx, %ss

	/* ltr %di */
    
	xorl %ecx, %ecx
	xorl %edx, %edx
	xorl %ebx, %ebx
	xorl %ebp, %ebp
	xorl %edi, %edi

	/* lldt %cx */

	jmpl *%eax

/* end in_pm32 */
