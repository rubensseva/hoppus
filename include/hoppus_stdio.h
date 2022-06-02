#ifndef HOPPUS_STDIO_H_
#define HOPPUS_STDIO_H_

#include <hoppus_config.h>

#ifdef HOPPUS_RISCV_F9

#include <user_stdio.h>
#include <user_thread.h>
#include <user_thread_log.h>

/* #define hoppus_printf(str, ...) {               \ */
/*         user_printf(str, __VA_ARGS__);          \ */
/*     } */
#define hoppus_printf(str, ...) {               \
        user_log_printf(str, __VA_ARGS__);          \
    }
/* #define hoppus_puts(str) { \ */
/*         user_puts(str);    \ */
/*     } */
#define hoppus_puts(str) { \
        user_log_puts(str);    \
    }
#define hoppus_read_line(str) { \
        read_line(buf);         \
    }

#endif
#ifdef HOPPUS_X86

#include <stdio.h>
#include <stdlib.h>
#define hoppus_printf(str, ...) { \
        printf(str, __VA_ARGS__); \
    }
#define hoppus_puts(str) { \
        printf(str);       \
    }
#define hoppus_read_line(str) {     \
        fgets(buf, 1000, stdin);    \
    }

#endif

#endif // HOPPUS_STDIO_H_
