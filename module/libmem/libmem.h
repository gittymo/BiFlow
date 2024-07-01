#ifndef COM_PLUS_MEVANSPN_LIBMEM
#define COM_PLUS_MEVANSPN_LIBMEM

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void LibMemAllowLocals();
void * LibMemAlloc(size_t required_size_in_bytes, void ** ref_ptr);
void * LibMemAllocLocal(size_t required_size_in_bytes, void ** ref_ptr);
bool LibMemResize(void * ptr, size_t new_size_bytes);
bool LibMemFree(void * ptr);
bool LibMemFreeLocals();

#endif