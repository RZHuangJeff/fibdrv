#include "xs.h"

static inline bool xs_malloc(xs *x, size_t len)
{
    x->capacity = ilog2(len) + 1;

    if (len <= XS_LARGE_STR_LEN)
        x->ptr = xs_is_ptr(x)
                     ? krealloc(x->ptr, (size_t) 1 << x->capacity, GFP_KERNEL)
                     : kmalloc((size_t) 1 << x->capacity, GFP_KERNEL);
    else {
        x->ptr =
            xs_is_ptr(x)
                ? krealloc(x->ptr, ((size_t) 1 << x->capacity) + 4, GFP_KERNEL)
                : kmalloc(((size_t) 1 << x->capacity) + 4, GFP_KERNEL);
        x->is_large_str = 1;
    }
    x->is_ptr = 1;

    return x->ptr != NULL;
}

xs *xs_new(xs *x, const void *p)
{
    *x = xs_empty();
    size_t len = strlen(p) + 1;
    if (len <= 16) {
        memcpy(x->data, p, len);
        x->space_left = 16 - len;
    } else {
        xs_grow(x, len);
        memcpy(x->ptr, p, len);
        x->size = len - 1;
    }

    return x;
}

void xs_free(xs *x)
{
    if (xs_is_ptr(x) || (xs_is_large_string(x) && !xs_dec_ref_cnt(x)))
        kfree(x->ptr);

    *x = xs_empty();
}

xs *xs_grow(xs *x, size_t len)
{
    char buf[16] = {0};
    if (len < xs_capacity(x))
        return x;

    if (!xs_is_ptr(x)) {
        memcpy(buf, x->data, sizeof(xs));
        x->size = 15 - x->space_left;
    }

    bool res = xs_malloc(x, len);

    if (res && buf[0])
        memcpy(x->ptr, buf, sizeof(xs));

    return x;
}