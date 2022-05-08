#ifndef HOPPUS_LINK_H_
#define HOPPUS_LINK_H_

#include <hoppus_config.h>

#ifdef HOPPUS_RISCV_F9
    #include <link.h>
#endif
#ifdef HOPPUS_X86
    #define __USER_TEXT         __attribute__ (())
    #define __USER_DATA         __attribute__ (())
#endif

/* #define __USER_TEXT         __attribute__ (()) */
/* #define __USER_DATA         __attribute__ (()) */
#endif // HOPPUS_LINK_H_
