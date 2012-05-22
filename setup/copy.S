.code16gcc
.section ".text"
.globl memset
memset:
    pushw %di
	movw %ax, %di
	movzbl %dl, %eax
	imull $0x01010101, %eax
	pushw %cx
	shrw $2, %cx
	rep; stosl
	popw %cx
	andw $3, %cx
	rep; stosb
	popw %di
	ret
