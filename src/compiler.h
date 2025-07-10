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

#include <stdio.h>

#define xopen(...) open(__VA_ARGS__)
#define xread(h, d, l) read(h, d, l)
#define xlseek(h, o, m) lseek(h, o, m)
#define xtell(h) tell(h)
#define xwrite(h, d, l) write(h, d, l)
#define xclose(h) close(h)

#define xfopen(f, m)  fopen(f, m)
#define xfread(f, s, c, l) fread(f, s, c, l)
#define xfseek(h, o, m) fseek(h, o, m)
#define xftell(h) ftell(h)
#define xfwrite(h, d, l) fwrite(h, d, l)
#define xfclose(h) fclose(h)


#endif // __COMPILER_H__
