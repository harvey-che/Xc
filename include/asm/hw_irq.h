#ifndef _ASM_X86_HW_IRQ_H
#define _ASM_X86_HW_IRQ_H

#include <Xc/atomic.h>
#include <asm/irq_vectors.h>
#include <Xc/percpu.h>

extern atomic_t irq_err_count;
extern atomic_t irq_mis_count;

typedef int vector_irq_t[NR_VECTORS];
DECLARE_PER_CPU(vector_irq_t, vector_irq);

extern void (* interrupt[NR_VECTORS - FIRST_EXTERNAL_VECTOR])(void);

#endif
