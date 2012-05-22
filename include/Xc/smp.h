#ifndef _XC_SMP_H
#define _XC_SMP_H

#define raw_smp_processor_id() 0
#define smp_processor_id() raw_smp_processor_id()

#endif
