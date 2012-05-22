#ifndef _ASM_X86_PGALLOC_H

#include <Xc/mm_types.h>

#ifndef __ASSEMBLY__

extern pgd_t *pgd_alloc(struct mm_struct *);

extern void pgd_free(struct mm_struct *mm, pgd_t *pgd);

#endif

#endif
