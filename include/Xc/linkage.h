#ifndef _XC_LINKAGE_H
#define _XC_LINKAGE_H

#define __weak __attribute__((weak))

#define ALIGN .align 4, 0x90

#ifdef __ASSEMBLY__

#ifndef ENTRY
#define ENTRY(name)    \
	.globl name;    \
    ALIGN;     \
    name:
#endif

#ifndef END
#define END(name)    \
	.size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name)    \
	.type name, @function;    \
    END(name)
#endif

#endif

#endif
