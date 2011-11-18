#include <Xc/linkage.h>
#include <asm/irq_vectors.h>
#include <asm/segment.h>
#include <Xc/thread_info.h>
#include <asm/processor-flags.h>

#define PT_EBX 0
#define PT_ECX 4
#define PT_EDX 8
#define PT_ESI 12
#define PT_EDI 16
#define PT_EBP 20
#define PT_EAX 24
#define PT_DS  28
#define PT_ES  32
#define PT_FS  36
#define PT_GS  40
#define PT_ORIG_EAX 44
#define PT_EIP 48
#define PT_CS  52
#define PT_EFLAGS 56
#define PT_OLDESP 60
#define PT_OLDSS  64

#define TI_task 0
#define TI_exec_domain 4
#define TI_flags 8
#define TI_status 12
#define TI_preempt_count 20
#define TI_addr_limit 24
#define TI_restart_block 28
#define TI_sysenter_return 60
#define TI_cpu 16

#define CLBR_ANY ((1 << 9) - 1)

#define preempt_stop(clobbers) DISABLE_INTERRUPTS(clobbers); TRACE_IRQS_OFF

#define resume_userspace_sig resume_userspace

#define ENABLE_INTERRUPTS(x) sti
#define DISABLE_INTERRUPTS(x) cli

#define INTERRUPT_RETURN iret

#define TRACE_IRQS_ON
#define TRACE_IRQS_OFF

.macro TRACE_IRQS_IRET
.endm

#define LOCKDEP_SYS_EXIT

#define CFI_ADJUST_CFA_OFFSET
#define CFI_REMEMBER_STATE
#define CFI_RESTORE_STATE


.macro pushl_cfi reg
    pushl \reg
	/* CFI_ADJUST_CFA_OFFSET 4 */
.endm

.macro popl_cfi reg
    popl \reg
	/* CFI_ADJUST_CFA_OFFSET -4 */
.endm

.macro PUSH_GS
    pushl_cfi %gs
.endm

.macro POP_GS pop=0
    popl_cfi %gs
	.if \pop <> 0
	    add $\pop, %esp
		/* CFI_ADJUST_CFA_OFFSET -\pop */
	.endif
.endm

.macro POP_GS_EX
    
.endm

.macro SET_KERNEL_GS reg
    movl $(__KERNEL_STACK_CANARY), \reg
	movl \reg, %gs
.endm

.macro SAVE_ALL
    cld
	PUSH_GS
	pushl_cfi %fs
	pushl_cfi %es
	pushl_cfi %ds
	pushl_cfi %eax
	pushl_cfi %ebp
	pushl_cfi %edi
	pushl_cfi %esi
	pushl_cfi %edx
	pushl_cfi %ecx
	pushl_cfi %ebx
	movl $(__USER_DS), %edx
	movl %edx, %ds
	movl %edx, %es
	movl $(__KERNEL_PERCPU), %edx
	movl %edx, %fs
	SET_KERNEL_GS %edx
.endm

.macro RESTORE_INT_REGS
    popl_cfi %ebx
	popl_cfi %ecx
	popl_cfi %edx
	popl_cfi %esi
	popl_cfi %edi
	popl_cfi %ebp
	popl_cfi %eax
.endm

.macro RESTORE_REGS pop=0
    RESTORE_INT_REGS
1:  popl_cfi %ds
2:  popl_cfi %es
3:  popl_cfi %fs
    POP_GS \pop
    POP_GS_EX
.endm

.section ".data"
ENTRY(interrupt)
.section ".text", "ax"
.p2align 5
ENTRY(irq_entries_start)
vector = FIRST_EXTERNAL_VECTOR
.rept (NR_VECTORS - FIRST_EXTERNAL_VECTOR + 6) / 7
    .balign 32
    .rept 7
	    .if vector < NR_VECTORS
		    .if vector <> FIRST_EXTERNAL_VECTOR
			    /* CFI_ADJUST_CFA_OFFSET -4 */
			.endif
1:              pushl_cfi $(~vector + 0x80)
            .if ((vector - FIRST_EXTERNAL_VECTOR) % 7) <> 6
	            jmp 2f
	        .endif
	        .previous
	        .long 1b
	        .section ".text", "ax"
vector=vector+1
        .endif
    .endr
2:  jmp common_interrupt
.endr
END(irq_entries_start)
.previous
END(interupt)
.previous

.p2align 5
common_interrupt:
    addl $-0x80, (%esp)
	SAVE_ALL
	TRACE_IRQS_OFF
	movl %esp, %eax
	call do_IRQ
	jmp ret_from_intr
ENDPROC(common_interrupt)

    ALIGN
ret_from_exception:
    preempt_stop(CLBR_ANY)
ret_from_intr:
    GET_THREAD_INFO(%ebp)
check_userspace:
    movl PT_EFLAGS(%esp), %eax
	movb PT_CS(%esp), %al
	andl $(X86_EFLAGS_VM | SEGMENT_RPL_MASK), %eax
	cmpl $USER_RPL, %eax
	jb resume_kernel
ENTRY(resume_userspace)
    LOCKDEP_SYS_EXIT
	DISABLE_INTERRUPTS(CLBR_ANY)

	TRACE_IRQS_OFF
	movl TI_flags(%ebp), %ecx
	andl $_TIF_WORK_MASK, %ecx

	jne work_pending
	jmp restore_all
END(ret_from_exception)

ENTRY(resume_kernel)
    DISABLE_INTERRUPTS(CLBR_ANY)
	cmpl $0, TI_preempt_count(%ebp)
	jnz restore_all
need_resched:
    movl TI_flags(%ebp), %ecx
	testb $_TIF_NEED_RESCHED, %cl
	jz restore_all
	testl $X86_EFLAGS_IF, PT_EFLAGS(%esp)
	jz restore_all
	call preempt_schedule_irq
	jmp need_resched
END(resume_kernel)

restore_all:
    TRACE_IRQS_IRET
restore_all_notrace:
    movl PT_EFLAGS(%esp), %eax

    /* See process.c:copy_thread() for details */
	movb PT_OLDSS(%esp), %ah
	movb PT_CS(%esp), %al
	andl $(X86_EFLAGS_VM | (SEGMENT_TI_MASK << 8) | SEGMENT_RPL_MASK), %eax
	cmpl $((SEGMENT_LDT << 8) | USER_RPL), %eax
	CFI_REMEMBER_STATE
	je ldt_ss
restore_nocheck:
    RESTORE_REGS 4
irq_return:
    INTERRUPT_RETURN
	CFI_RESTORE_STATE
ldt_ss:
    /* Not support these right now*/
    jmp ldt_ss
	
	ALIGN
	/* RING0_PTREGS_FRAME */
work_pending:
    testb $_TIF_NEED_RESCHED, %cl
	jz work_notifysig
work_resched:
    call schedule
	LOCKDEP_SYS_EXIT
	DISABLE_INTERRUPTS(CLBR_ANY)

	TRACE_IRQS_OFF
	movl TI_flags(%ebp), %ecx
	andl $_TIF_WORK_MASK, %ecx

	jz restore_all
	testb $_TIF_NEED_RESCHED, %cl
	jnz work_resched

work_notifysig:
    movl %esp, %eax
	
	xorl %edx, %edx
	/* By harvey, Signal is not supported yet */
	/* call do_notify_resume */
	jmp resume_userspace_sig
END(work_pending)
