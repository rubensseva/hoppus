#ifndef HOPPUS_LINK_H_
#define HOPPUS_LINK_H_

#include <hoppus_config.h>

#ifndef HOPPUS_PLATFROM
    #include <link.h>
#elif HOPPUS_PLATFORM == HOPPUS_RISCV_F9
    #include <link.h>
#elif HOPPUS_PLATFORM == HOPPUS_X86
    #define __USER_TEXT         __attribute__ ((section(".user_text")))
    #define __USER_DATA         __attribute__ ((section(".user_data")))
#endif

#define __USER_TEXT         __attribute__ (())
#define __USER_DATA         __attribute__ (())
#endif // HOPPUS_LINK_H_
