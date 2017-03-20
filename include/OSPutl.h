#ifndef __OSPUTL_H__
#define __OSPUTL_H__

void OSPrint(uint64_t, const char *, ...);
uint32_t OSPrand(uint32_t);
void *OSPArray(size_t, ...);

#define RANDOM(seed) ((((seed) = OSPrand(seed)) >> 16) & ~0x8000)

#endif /* __OSPUTL_H__ */

