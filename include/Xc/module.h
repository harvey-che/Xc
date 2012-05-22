#ifndef _XC_MODULE_H
#define _XC_MODULE_H

struct module
{

};

#ifdef CONFIG_MODULES

#else

static inline int try_module_get(struct module *modle) { return 1; }
static inline void module_put(struct module *module)
{
}

#endif

#endif
