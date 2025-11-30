#ifndef OKO_TEMP_ALLOCATOR_H
#define OKO_TEMP_ALLOCATOR_H

#include "st.h"
#include <stdlib.h>
#include <assert.h>

typedef struct {
    u8* buffer;
    u64 capacity;
    u64 offset;
} oko_temp_allocator;

typedef struct {
    u64 offset;
} oko_temp_marker;


static oko_temp_allocator oko_temp_init(u64 capacity);
static void oko_temp_destroy(oko_temp_allocator* alloc);
static void* oko_temp_alloc(oko_temp_allocator* alloc, u64 size, u64 align);
static oko_temp_marker oko_temp_get_marker(const oko_temp_allocator* alloc);
static void oko_temp_reset_to_marker(oko_temp_allocator* alloc, oko_temp_marker marker);

static inline u64 oko_temp_bytes_used(const oko_temp_allocator* alloc) {
    return alloc->offset;
}
static inline u64 oko_temp_bytes_available(const oko_temp_allocator* alloc) {
    return alloc->capacity - alloc->offset;
}
static inline u64 oko_temp_capacity(const oko_temp_allocator* alloc) {
    return alloc->capacity;
}
static inline void oko_temp_reset(oko_temp_allocator* alloc) {
    alloc->offset = 0;
}
static inline void* oko_temp_alloc_default(oko_temp_allocator* alloc, u64 size) {
    return oko_temp_alloc(alloc, size, 8);
}

#define oko_temp_alloc_type(alloc, type, count) \
    ((type*)oko_temp_alloc(alloc, sizeof(type) * (count), _Alignof(type)))

#ifdef __GNUC__
#define oko_temp_scope(alloc) \
for (oko_temp_marker _oko_marker = oko_temp_get_marker(alloc), _oko_done = {0}; \
!_oko_done.offset; \
oko_temp_reset_to_marker(alloc, _oko_marker), _oko_done.offset = 1)
#endif


#ifdef OKO_TEMP_ALLOCATOR_IMPLEMENTATION

static oko_temp_allocator oko_temp_init(u64 capacity) {
    oko_temp_allocator alloc;
    alloc.buffer = (u8*)malloc(capacity);
    alloc.capacity = capacity;
    alloc.offset = 0;
    assert(alloc.buffer && "Failed to allocate memory");
    return alloc;
}
static void oko_temp_destroy(oko_temp_allocator* alloc) {
    free(alloc->buffer);
    alloc->buffer = NULL;
    alloc->offset = 0;
    alloc->capacity = 0;
}
static void* oko_temp_alloc(oko_temp_allocator* alloc, u64 size, u64 align) {
    u64 aligned_offset = (alloc->offset + align - 1) & ~(align - 1);

    if (aligned_offset + size > alloc->capacity) {
        return NULL;
    }

    void* ptr = alloc->buffer + aligned_offset;
    alloc->offset = aligned_offset + size;
    return ptr;
}
static oko_temp_marker oko_temp_get_marker(const oko_temp_allocator* alloc) {
    oko_temp_marker marker;
    marker.offset = alloc->offset;
    return marker;
}
static void oko_temp_reset_to_marker(oko_temp_allocator* alloc, oko_temp_marker marker) {
    assert(marker.offset <= alloc->offset);
    alloc->offset = marker.offset;
}

#endif
#endif // OKO_TEMP_ALLOCATOR_H
