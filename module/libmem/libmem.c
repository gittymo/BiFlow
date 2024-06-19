/*  libmem.c
    Memory manager for Biflow module
    (C)2024 Morgan Evans */

#ifndef COM_PLUS_MEVANSPN_BIFLOW_MEM
#define COM_PLUS_MEVANSPN_BIFLOW_MEM

#include <stdlib.h>

#define LIBMEM_DATA_STRUCT_ID (('L' << 24) + ('M' << 16) + ('e' << 8) + 'm')
#define LIBMEM_BACKING_BLOCK_SIZE (4 * 1024 * 1024)

typedef struct libmem_allocation {
    int struct_id;
    size_t offset;
    size_t length;
    void ** ref_var_ptr;
} LibmemAllocation;

typedef struct libmem_manager {
    int struct_id;
    char * data;
    size_t data_size;
    size_t data_used;
    LibmemAllocation ** allocation_tables[256];
    size_t allocation_table_sizes[256];
    size_t allocation_table_block_sizes[256];
} LibmemManager;

LibmemManager * LibmemCreateManager()
{
    LibmemManager * mman = (LibmemManager *) malloc(sizeof(LibmemManager));
    if (mman) {
        mman->struct_id = LIBMEM_DATA_STRUCT_ID;
        mman->data = (char *) malloc(sizeof(LIBMEM_BACKING_BLOCK_SIZE));
        mman->data_size = LIBMEM_BACKING_BLOCK_SIZE;
        for (int i = 0; i < 256; i++) {
            mman->allocation_tables[i] = (LibmemAllocation **) calloc(sizeof(LibmemAllocation *) * 256);
            mman->allocation_table_sizes[i] = 0;
            mman->allocation_table_block_sizes[i] = 256;
        }
    }
    return mman;
}

void LibmemResizeBc

void * LibmemAllocate(LibmemManager * mman, size_t size_in_bytes, void ** ref_var_ptr)
{
    if (!mman || mman->struct_id != LIBMEM_DATA_STRUCT_ID || size_in_bytes < 1 || !ref_var_ptr) return NULL;
    if (size_in_bytes > mman->data_size - mman->data_used) {
        size_t new_data_size = mman->data_size;
        while (mman->data_size <= new_data_size) mman->data_size += LIBMEM_BACKING_BLOCK_SIZE;
        mman->data = (char *) realloc(mman->data, mman->data_size);
        for (int i = 0; i < 256; i++) {
            LibmemAllocation ** alloc_table = mman->allocation_tables[i];
            size_t alloc_table_size = mman->allocation_table_sizes[i];
            for (int j = 0; j < alloc_table_size; j++) {
                LibmemAllocation * alloc = alloc_table[j];
                if (alloc) {
                    *alloc->ref_var_ptr = mman->data + alloc->offset;
                }
            }
        }
    }
}

#endif