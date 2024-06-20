#include "libmem.h"

#define LIBMEM_BLOCK_SIZE 256
#define LIBMEM_MEMORY_PAGE_SIZE (LIBMEM_BLOCK_SIZE * 256)
#define LIBMEM_DATA_STRUCT_ID (('L' << 24) + ('M' << 16) + ('e' << 8) + 'm')

struct _libmem_memory_block;

typedef struct libmem_allocation {
    int struct_id;
    size_t offset;
    size_t length;
    size_t length_in_block_bytes;
    struct _libmem_memory_block * memory_block;
    void *** references;
    size_t references_count;
    size_t references_buffer_size;
    bool is_local;
    size_t stack_level;
} MemoryAllocation;

typedef struct _libmem_memory_block {
    int struct_id;
    char * data;
    size_t data_size;
    MemoryAllocation ** allocations;
    size_t allocations_count;
    size_t allocations_buffer_size;
    size_t current_stack_level;
} MemoryBlock;

typedef struct _libmem_allocation_ref {
    int struct_id;
    MemoryBlock * memory_block;
    size_t allocation_index;
} MemoryAllocationReference;

static MemoryBlock * libmem_manager = NULL;

void LibMemExit();

size_t LibMemGetClosestAlignedByteSize(size_t required_bytes, size_t unit_size_bytes)
{
    if (unit_size_bytes == 0) unit_size_bytes = LIBMEM_BLOCK_SIZE;
    const size_t FLOOR_REQUIRED_BYTES = (required_bytes / unit_size_bytes) * unit_size_bytes;
    return FLOOR_REQUIRED_BYTES + (unit_size_bytes * (FLOOR_REQUIRED_BYTES < required_bytes));
}

bool LibMem_IsValidMemoryBlock(MemoryBlock * m)
{
    return m && m->struct_id == LIBMEM_DATA_STRUCT_ID;
}

bool LibMem_IsValidMemoryAllocation(MemoryAllocation * ma)
{
    return ma && ma->struct_id == LIBMEM_DATA_STRUCT_ID;
}

MemoryAllocation * LibMemCreateMemoryAllocation(size_t offset, size_t required_size_in_bytes, MemoryBlock * memory_block)
{
    MemoryAllocation * new_alloc = (MemoryAllocation *) malloc(sizeof(MemoryAllocation));
    if (new_alloc) {
        new_alloc->struct_id = LIBMEM_DATA_STRUCT_ID;
        new_alloc->offset = offset;
        new_alloc->length = required_size_in_bytes;
        new_alloc->length_in_block_bytes = LibMemGetClosestAlignedByteSize(required_size_in_bytes, LIBMEM_BLOCK_SIZE);
        new_alloc->memory_block = memory_block;
        new_alloc->references_buffer_size = 8;
        new_alloc->references_count = 0;
        new_alloc->references = (void ***) malloc(sizeof(void**) * new_alloc->references_buffer_size);

        MemoryAllocationReference * mar = (MemoryAllocationReference *) malloc(sizeof(MemoryAllocationReference));
        mar->struct_id = LIBMEM_DATA_STRUCT_ID;
        mar->memory_block = memory_block;
        mar->allocation_index = memory_block->allocations_count;

        MemoryAllocationReference ** mar_ptr = (MemoryAllocationReference **) (memory_block->data + new_alloc->offset);
        *mar_ptr = mar;

        memory_block->allocations[memory_block->allocations_count++] = new_alloc;
    }

    return new_alloc;
}

MemoryBlock * LibMemCreateMemoryBlock()
{
    if (libmem_manager) return libmem_manager;
    libmem_manager = (MemoryBlock *) malloc(sizeof(MemoryBlock));
    if (libmem_manager) {
        libmem_manager->struct_id = LIBMEM_DATA_STRUCT_ID;
        libmem_manager->data = (char *) malloc(LIBMEM_MEMORY_PAGE_SIZE);
        libmem_manager->data_size = LIBMEM_MEMORY_PAGE_SIZE;
        libmem_manager->allocations_buffer_size = 256;
        libmem_manager->allocations = (MemoryAllocation **) malloc(sizeof(MemoryAllocation *) * 256);
        libmem_manager->allocations_count = 0;
        libmem_manager->current_stack_level = 0;
    }
    return libmem_manager;
}

MemoryAllocation * LibMemGetMemoryAllocationFromVoidPtr(void * ptr, size_t * allocation_index_ptr)
{
    MemoryAllocationReference ** mar_ptr = (MemoryAllocationReference **) ((char *) ptr - sizeof(MemoryAllocationReference **));
    MemoryAllocationReference * mar = *mar_ptr;
    if (mar && mar->struct_id == LIBMEM_DATA_STRUCT_ID && allocation_index_ptr) *allocation_index_ptr = mar->allocation_index;
    return (!mar || mar->struct_id != LIBMEM_DATA_STRUCT_ID) ? NULL : mar->memory_block->allocations[mar->allocation_index];
}

void * LibMemRef(void * reference_this_ptr, void ** with_this_var_ptr)
{
    if (!reference_this_ptr || !with_this_var_ptr) return NULL;
    MemoryAllocation * alloc = LibMemGetMemoryAllocationFromVoidPtr(reference_this_ptr, NULL);
    if (!LibMem_IsValidMemoryAllocation(alloc)) return NULL;
    bool ref_found = false;
    for (size_t i = 0; i < alloc->references_count && !ref_found; i++) ref_found = alloc->references[i] == with_this_var_ptr;
    if (!ref_found) {
        if (alloc->references_count == alloc->references_buffer_size) {
            alloc->references_buffer_size += 8;
            alloc->references = (void ***) realloc(alloc->references, sizeof(void **) * alloc->references_buffer_size);
        }
        alloc->references[alloc->references_count++] = with_this_var_ptr;
    }
    *with_this_var_ptr = reference_this_ptr;
    return reference_this_ptr;
}

void LibMemUpdateRefs(MemoryBlock * libmem_manager)
{
    if (!LibMem_IsValidMemoryBlock(libmem_manager)) return;
    for (size_t a = 0; a < libmem_manager->allocations_count; a++) {
        MemoryAllocation * ma = libmem_manager->allocations[a];
        if (!ma) continue;
        for (size_t r = 0; r < libmem_manager->allocations[a]->references_count; r++) {
            void ** ref = ma->references[r];
            if (!ref) continue;
            *ref = libmem_manager->data + ma->offset + sizeof(MemoryAllocation **);        
        }
    }
}

void * LibMemAlloc(size_t required_size_in_bytes, void ** ref_ptr)
{
    if (!libmem_manager) {
        libmem_manager = LibMemCreateMemoryBlock();
        if (!libmem_manager) return NULL;
        else atexit(LibMemExit);
    }

    if (!libmem_manager || required_size_in_bytes == 0) return NULL;
    size_t required_size_in_block_bytes = LibMemGetClosestAlignedByteSize(required_size_in_bytes, LIBMEM_BLOCK_SIZE);
    if (libmem_manager->allocations_count == libmem_manager->allocations_buffer_size) {
        libmem_manager->allocations_buffer_size += 256;
        libmem_manager->allocations = (MemoryAllocation **) realloc(libmem_manager->allocations, sizeof(MemoryAllocation *) * libmem_manager->allocations_buffer_size);
    }

    size_t total_free = libmem_manager->data_size;
    size_t space_between_blocks = 0;
    size_t o = 0;
    bool allocated_block = false;
    MemoryAllocation * new_alloc = NULL;
    for (size_t i = 0; i < libmem_manager->allocations_count && !allocated_block; i++) {
        MemoryAllocation * tmp_alloc = libmem_manager->allocations[i];
        if (tmp_alloc) {
            space_between_blocks = tmp_alloc->offset - o;
            total_free -= tmp_alloc->length_in_block_bytes;
            if (space_between_blocks >= required_size_in_block_bytes) {
                new_alloc = LibMemCreateMemoryAllocation(o, required_size_in_bytes, libmem_manager);
                if (new_alloc) allocated_block = true;
            }
            o = tmp_alloc->offset + tmp_alloc->length_in_block_bytes;
        }
    }

    if (!allocated_block) {
        if (libmem_manager->data_size - o >= required_size_in_block_bytes) {
            new_alloc = LibMemCreateMemoryAllocation(o, required_size_in_bytes, libmem_manager);
        } else {
            if (total_free <= required_size_in_block_bytes) {
                libmem_manager->data_size = LibMemGetClosestAlignedByteSize(o + required_size_in_block_bytes, LIBMEM_MEMORY_PAGE_SIZE);
                libmem_manager->data = (char *) realloc(libmem_manager->data, libmem_manager->data_size);
                LibMemUpdateRefs(libmem_manager);
            }
            new_alloc = LibMemCreateMemoryAllocation(o, required_size_in_bytes, libmem_manager);
        }
    }

    void * return_pointer = (void *) new_alloc ? libmem_manager->data + new_alloc->offset + sizeof(MemoryAllocation *) : NULL;
    return LibMemRef(return_pointer, ref_ptr);
}

void * LibMemAllocLocal(size_t required_size_in_bytes, void ** ref_ptr) 
{
    void * ptr = LibMemAlloc(required_size_in_bytes, ref_ptr);
    if (!ptr) return NULL;

    MemoryAllocation * ma = LibMemGetMemoryAllocationFromVoidPtr(ptr, NULL);
    ma->is_local = true;
    ma->stack_level = libmem_manager->current_stack_level;
    return ptr;
}

void LibMemAllowLocals() 
{
    libmem_manager->current_stack_level++;
}

bool LibMemFree(void * ptr)
{
    if (!ptr) return false;
    size_t allocation_index;
    MemoryAllocation * ma = LibMemGetMemoryAllocationFromVoidPtr(ptr, &allocation_index);
    if (!LibMem_IsValidMemoryAllocation(ma)) return false;
    MemoryBlock * mb = ma->memory_block;
    if (!LibMem_IsValidMemoryBlock(mb)) return false;

    for (size_t r = 0; r < ma->references_count; r++) {
        *ma->references[r] = NULL;
        ma->references[r] = NULL;
    }
    free(ma->references);
    ma->references = NULL;
    ma->references_count = ma->references_buffer_size = 0;
    ma->length = ma->length_in_block_bytes = 0;
    ma->memory_block = NULL;
    ma->offset = 0;
    ma->struct_id = 0;
    free(ma);

    for (size_t a = allocation_index; a < mb->allocations_count - 1; a++) {
        mb->allocations[a] = mb->allocations[a + 1];
        MemoryAllocationReference ** mar_ptr = (MemoryAllocationReference **) (mb->data + mb->allocations[a + 1]->offset);
        MemoryAllocationReference * mar = *mar_ptr;
        mar->allocation_index--;
        mb->allocations[a + 1] = NULL;
    }

    mb->allocations_count--;

    for (size_t i = 0; i < mb->allocations_count; i++) {
        for (size_t j = mb->allocations_count - 1; j > i; j--) {
            if (mb->allocations[i]->offset > mb->allocations[j]->offset) {
                MemoryAllocation * ta = mb->allocations[i];
                MemoryAllocationReference ** ir_ptr = (MemoryAllocationReference **) (mb->data + mb->allocations[i]->offset);
                MemoryAllocationReference ** jr_ptr = (MemoryAllocationReference **) (mb->data + mb->allocations[i]->offset);
                (*jr_ptr)->allocation_index = i;
                (*ir_ptr)->allocation_index = j;
                mb->allocations[i] = mb->allocations[j];
                mb->allocations[j] = ta;
            }
        }
    }

    return true;
}

bool LibMemFreeLocals()
{
    if (!LibMem_IsValidMemoryBlock(libmem_manager)) return false;
    size_t local_ptrs_count = 256, i = 0;
    void ** local_ptrs = (void **) malloc(sizeof(void *) * local_ptrs_count);
    for (size_t a = 0; a < libmem_manager->allocations_count; a++) {
        MemoryAllocation * ma = libmem_manager->allocations[a];
        if (ma->is_local && ma->stack_level == libmem_manager->current_stack_level) {
            local_ptrs[i++] = (void *) (libmem_manager->data + ma->offset + sizeof(MemoryAllocationReference **));
            if (i == local_ptrs_count) {
                local_ptrs_count += 256;
                local_ptrs = (void **) realloc(local_ptrs, sizeof(void *) * local_ptrs_count);
            }
        }
    }
    for (size_t l = 0; l < i; l++) {
        LibMemFree(local_ptrs[l]);
    }
    if (libmem_manager->current_stack_level > 0) libmem_manager->current_stack_level--;
    return true;
}

void LibMemExit()
{
    if (!LibMem_IsValidMemoryBlock(libmem_manager)) return;
    while (libmem_manager->allocations_count > 0) {
        void * ptr = (void *) (libmem_manager->data + libmem_manager->allocations[0]->offset + sizeof(MemoryAllocationReference **));
        LibMemFree(ptr);
    }
    free(libmem_manager->allocations);
    libmem_manager->allocations = NULL;
    libmem_manager->allocations_buffer_size = 0;
    free(libmem_manager->data);
    libmem_manager->data = NULL;
    libmem_manager->data_size = 0;
    libmem_manager->struct_id = 0;
    free(libmem_manager);
    libmem_manager = NULL;
}