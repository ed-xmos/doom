#ifndef __COMPILER_H__
#define __COMPILER_H__

#if defined(USE_EXTMEM)
#define OVERLAY __attribute__ ((section(".ExtMem.code")))
#define EXTMEMD __attribute__ ((section(".ExtMem.bss")))
#define EXTMEMDI __attribute__ ((section(".ExtMem.data")))
#else
#define OVERLAY
#define EXTMEMD
#define EXTMEMDI
#endif

#include "xfile.h"

#endif // __COMPILER_H__
