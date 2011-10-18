#include <asm/pgtable_types.h>
#include <asm/segment.h>
#include <asm/processor-flags.h>


#define pa(x) ((x) - PAGE_OFFSET)

LOWMEM_PAGES = (((1 << 32) - PAGE_OFFSET) >> PAGE_SHIFT)

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
	leal -PAGE_OFFSET(%ecx), %esp

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
	movl $128, %ecx
	cld
	rep; movsl

    /* Construct provisional page tables */
page_pde_offset = (PAGE_OFFSET >> 20)

    movl $pa(__brk_base), %edi
	movl $pa(initial_page_table), %edx
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

	addl $PAGE_OFFSET, %edi
	movl %edi, pa(_brk_end)
	shrl $12, %eax
	movl %eax, pa(max_pfn_mapped)

	movl $pa(initial_page_table), %eax
	movl %eax, %cr3
	movl %cr0, %eax
	orl $X86_CR0_PG, %eax
	movl %eax, %cr0
	ljmp $__BOOT_CS, $1f
1:
    addl $PAGE_OFFSET, %esp

    pushl $0
	popf

	call setup_idt

	xorl %eax, %eax
	lldt %ax
    
	pushl $0
	movl %esp, %ebp
    jmp start_kernel;

setup_idt:
    ret

    .section ".data"
    .align 4
	.word 0
boot_gdt_descr:
    .word __BOOT_DS + 7
	.long boot_gdt - PAGE_OFFSET

stack_start:
    .long init_thread_union + THREAD_SIZE
    
	.align 8
boot_gdt:
    .fill GDT_ENTRY_BOOT_CS, 8, 0
	.quad 0x00cf9a000000ffff
	.quad 0x00cf92000000ffff

.align 4096
.globl initial_page_table
initial_page_table:
    .fill 1024, 4, 0
.globl swapper_pg_dir
swapper_pg_dir:
    .fill 1024, 4, 0
