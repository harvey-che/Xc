#include <asm/pgtable_types.h>
#include <asm/segment.h>
#include <asm/processor-flags.h>
#include <asm/cache.h>


#define pa(x) ((x) - __PAGE_OFFSET)

LOWMEM_PAGES = (((1 << 32) - __PAGE_OFFSET) >> PAGE_SHIFT)

MAPPING_BEYOND_END = ((LOWMEM_PAGES / PTRS_PER_PGD) << PAGE_SHIFT)

.code32
.section ".text", "ax"
.globl startup_32
startup_32:
    lgdt pa(boot_gdt_descr)
	movw $__BOOT_DS, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

    /* Setup stack */
	movl pa(stack_start), %ecx
	leal -__PAGE_OFFSET(%ecx), %esp

    /* Clear bss */
    cld
	xorl %eax, %eax
	movl $pa(__bss_start), %edi
	movl $pa(__bss_stop), %ecx
	subl %edi, %ecx
	shrl $2, %ecx
	rep; stosl

    /* Copy boot_params */
    movl $pa(boot_params), %edi
	movl $1024, %ecx
	cld
	rep; movsl

    /* Construct provisional page tables */
page_pde_offset = (__PAGE_OFFSET >> 20)

    movl $pa(__brk_base), %edi
	movl $pa(swapper_pg_dir), %edx
	movl $PTE_IDENT_ATTR, %eax
1: 
    leal PDE_IDENT_ATTR(%edi), %ecx
	movl %ecx, (%edx)
	movl %ecx, page_pde_offset(%edx)
	addl $4, %edx
	movl $1024, %ecx
2:  stosl
    addl $0x1000, %eax
	loop 2b
	
    movl $pa(_end) + MAPPING_BEYOND_END + PTE_IDENT_ATTR, %ebp
	cmpl %ebp, %eax
	jb 1b

	addl $__PAGE_OFFSET, %edi
	movl %edi, pa(_brk_end)
	shrl $12, %eax
	movl %eax, pa(max_pfn_mapped)

/*
 * Enable paging
 */
	movl $pa(swapper_pg_dir), %eax
	movl %eax, %cr3
	movl %cr0, %eax
	orl $X86_CR0_PG, %eax
	movl %eax, %cr0
	ljmp $__BOOT_CS, $1f
1:
    addl $__PAGE_OFFSET, %esp

    pushl $0
	popf

	call setup_idt

    lgdt early_gdt_descr
    lidt idt_descr
	ljmp $(__KERNEL_CS), $1f
1:
    movl $(__KERNEL_DS), %eax
	movl %eax,  %ss
	movl $(__USER_DS), %eax
	movl %eax, %ds
	movl %eax, %es

	movl $(__KERNEL_PERCPU), %eax
	movl %eax, %fs

	movl $(__KERNEL_STACK_CANARY), %eax
	movl %eax, %gs

	xorl %eax, %eax
	lldt %ax
    
	pushl $0
	movl %esp, %ebp

    jmp start_kernel

setup_idt:
    leal ignore_int, %edx
	movl $(__KERNEL_CS << 16), %eax
	movw %dx, %ax
	movw $0x8E00, %dx

	leal idt_table, %edi
	mov $256, %ecx
rp_sidt:
    movl %eax, (%edi)
	movl %edx, 4(%edi)
	addl $8, %edi
	dec %ecx
	jne rp_sidt

.macro set_early_handler handler, trapno
    lea \handler, %edx
	movl $(__KERNEL_CS << 16), %eax
	movw %dx, %ax
	movw $0x8E00, %dx
	lea idt_table, %edi
	movl %eax, 8 * \trapno(%edi)
	movl %edx, 8 * \trapno + 4(%edi)
.endm
    
	set_early_handler handler=early_divide_err, trapno=0
	set_early_handler handler=early_illegal_opcode, trapno=6
	set_early_handler handler=early_protection_fault, trapno=13
	set_early_handler handler=early_page_fault, trapno=14
    /* set_early_handler handler=early_timer_handler, trapno=48	*/
	ret

.globl tick2
tick2:
    .byte 'B'
	.byte 0xF
early_timer_handler:
    pushl %eax
	incb tick2
	movw tick2, %ax
	movw %ax, 0xc00b8000
	movb $0x20, %al
	outb %al, $0x20
	popl %eax
    iret

early_divide_err:
    xor %edx, %edx
	push $0
	jmp early_default

early_illegal_opcode:
    mov $6, %edx
	pushl $0
	jmp early_default

early_protection_fault:
    movl $13, %edx
	jmp early_default

early_page_fault:
    movl $14, %edx
	jmp early_default

early_default:
    cld
hlt_loop:
    hlt
	jmp hlt_loop

    .align 4
ignore_int:
    cld
	iret


    .section ".data"
/* GDT and IDT */
    .align 4
	.word 0
boot_gdt_descr:
    .word __BOOT_DS + 7
	.long boot_gdt - __PAGE_OFFSET
    
	.word 0
.globl idt_descr
idt_descr:
    .word IDT_ENTRIES * 8 -1
	.long idt_table

    .word 0
.globl early_gdt_descr
early_gdt_descr:
    .word GDT_ENTRIES * 8 - 1
	.long gdt_page
    
	.align L1_CACHE_BYTES
boot_gdt:
    .fill GDT_ENTRY_BOOT_CS, 8, 0
	.quad 0x00cf9a000000ffff
	.quad 0x00cf92000000ffff

/* Page Table */
	.align 4096
/*
.globl initial_page_table
initial_page_table:
    .fill 1024, 4, 0
*/

.globl swapper_pg_dir
swapper_pg_dir:
    .fill 1024, 4, 0

/* initial thread stack */
	.align 4
stack_start:
    .long init_thread_union + THREAD_SIZE
    
