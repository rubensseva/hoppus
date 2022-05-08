#ifndef HOPPUS_TYPES_H_
#define HOPPUS_TYPES_H_

#include <hoppus_config.h>

#ifndef HOPPUS_PLATFORM
#elif HOPPUS_PLATFORM == HOPPUS_RISCV_F9
#include <types.h>
#elif HOPPUS_PLATFORM == HOPPUS_X86
#include <stddef.h>
#endif


#endif // HOPPUS_TYPES_H_
