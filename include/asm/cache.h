#ifndef _ASM_X86_CACHE_H
#define _ASM_X86_CACHE_H

#define L1_CACHE_SHIFT 5
#define L1_CACHE_BYTES (1 << L1_CACHE_SHIFT)

#define SMP_CACHE_BYTES L1_CACHE_BYTES

#define ____cacheline_aligned __attribute__((__aligned__(SMP_CACHE_BYTES)))

#define cache_line_size() L1_CACHE_BYTES

#define ____cacheline_aligned_in_smp

#define __cacheline_aligned_in_smp

#endif
