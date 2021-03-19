#ifndef XS_H
#define XS_H

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <stdbool.h>
#include <stddef.h>

#define XS_LARGE_STR_LEN 256

typedef union {
    char data[16];

    struct {
        uint8_t filler[15], space_left : 4, is_ptr : 1, is_large_str : 1,
            flags : 2;
    };

    struct {
        char *ptr;
        size_t size : 54, capacity : 6;
    };
} xs;

#define xs_empty() ((xs){.space_left = 15})

static inline bool xs_is_ptr(const xs *x)
{
    return x->is_ptr;
}

static inline bool xs_is_large_string(const xs *x)
{
    return x->is_large_str;
}

static inline size_t xs_size(const xs *x)
{
    return xs_is_ptr(x) ? x->size : 15 - x->space_left;
}

static inline size_t xs_capacity(const xs *x)
{
    return xs_is_ptr(x) ? ((size_t) 1 << x->capacity) - 1 : 15;
}

static inline char *xs_data(const xs *x)
{
    if (!xs_is_ptr(x))
        return (char *) x->data;

    return xs_is_large_string(x) ? (x->ptr + 4) : x->ptr;
}

static inline int xs_get_ref_cnt(const xs *x)
{
    if (!xs_is_large_string(x))
        return 0;

    return *(int *) (x->ptr);
}

static inline int xs_inc_ref_cnt(const xs *x)
{
    if (!xs_is_large_string(x))
        return 0;

    return ++(*(int *) (x->ptr));
}

static inline int xs_dec_ref_cnt(const xs *x)
{
    if (!xs_is_large_string(x))
        return 0;

    return --(*(int *) (x->ptr));
}

xs *xs_new(xs *x, const void *p);
void xs_free(xs *x);
xs *xs_grow(xs *x, size_t len);

#endif