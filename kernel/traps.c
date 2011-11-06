#include <asm/desc.h>
#include <asm/cache.h>
#include <asm/irq_vectors.h>

gate_desc idt_table[NR_VECTORS] __attribute__((aligned(L1_CACHE_BYTES))) = { { { { 0, 0 } } }, };
