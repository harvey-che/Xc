#ifndef _ASM_X86_PLATFORM_H
#define _ASM_X86_PLATFORM_H

struct x86_init_irqs {
    void (*pre_vector_init)(void);
	void (*intr_init)(void);
	void (*trap_init)(void);
};

struct x86_init_timers {
	//void (*setup_percpu_clockev)(void);
	//void (*tsc_pre_init)(void);
    void (*timer_init)(void);
	//void (*wallclock_init)(void);
};

struct x86_init_ops {
    //struct x86_init_resources resources;
	//struct x86_init_mpparse   mpparse;
	struct x86_init_irqs      irqs;
	//struct x86_init_oem       oem;
	//struct x86_init_mapping   mapping;
	//struct x86_init_paging    paging;
	struct x86_init_timers    timers;
	//struct x86_init_iommu     iommu;
	//struct x86_init_pci       pci;
};

extern struct x86_init_ops x86_init;

#endif
