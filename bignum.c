#include "bignum.h"

typedef struct {
    uint8_t dig_l : 4, dig_h : 4;
} bn_unit_t;

bn_t *bn_new(bn_t *bn, const char *p)
{
    size_t tot_len = strlen(p);
    size_t len = (tot_len + 1) >> 1;
    xs_new(bn, "");
    xs_grow(bn, len + 1);

    bn_unit_t *uptr = (bn_unit_t *) xs_data(bn);
    if (tot_len & 1) {
        uptr->dig_h = 0;
        uptr->dig_l = *p;
        uptr++;
        p++;
    }

    while (*p) {
        uptr->dig_h = *p;
        uptr->dig_l = *(p + 1);
        uptr++;
        p += 2;
    }
    *(char *) uptr = 0;

    if (xs_is_ptr(bn))
        bn->size = len;
    else
        bn->space_left = 15 - len;

    return bn;
}

bn_t *bn_add(const bn_t *a, const bn_t *b, bn_t *res)
{
    size_t a_size = xs_size(a);
    size_t b_size = xs_size(b);
    size_t max_size = a_size >= b_size ? a_size : b_size;
    size_t res_size = 0;

    bn_unit_t *a_data = (bn_unit_t *) xs_data(a);
    bn_unit_t *b_data = (bn_unit_t *) xs_data(b);

    xs_grow(res, max_size + 1);
    bn_unit_t *res_data = (bn_unit_t *) xs_data(res);

    uint8_t carry = 0;
    bn_unit_t null_unit = (bn_unit_t){.dig_h = 0, .dig_l = 0};
    *(char *) (res_data + max_size) = 0;
    for (ssize_t rid = max_size - 1, aid = a_size - 1, bid = b_size - 1;
         aid >= 0 || bid >= 0; rid--, aid--, bid--) {
        unit8_t n;
        bn_unit_t *_a = aid >= 0 ? a_data + aid : &null_unit;
        bn_unit_t *_b = bid >= 0 ? b_data + bid : &null_unit;

        n = _a->dig_l + _b->dig_l + carry;
        res_data[rid].dig_l = n % 10;
        carry = n / 10;

        n = _a->dig_h + _b->dig_h + carry;
        res_data[rid].dig_h = n % 10;
        carry = n / 10;

        res_size++;
    }

    if (carry) {
        xs_grow(res, ++res_size);
        res_data = (bn_unit_t *) xs_data(res);
        memmove(res_data + 1, res_data, res_size);
        res_data->dig_h = 0;
        res_data->dig_l = carry;
    }

    if (xs_is_ptr(res))
        res->size = res_size;
    else
        res->space_left = 15 - res_size;

    return res;
}

size_t bn_space_for_buf(const bn_t *bn)
{
    return xs_size(bn) * 2;
}

void bn_to_digit_string(const bn_t *bn, char *buf, size_t len)
{
    char *const limit = buf + len - 1;
    bn_unit_t *uptr = (bn_unit_t *) xs_data(bn);
    size_t ind = 0, size = xs_size(bn);
    if (!uptr[ind].dig_h) {
        *buf = uptr[ind].dig_l + '0';
        buf++;
        ind++;
    }

    while (ind < size && buf < limit) {
        *buf = uptr[ind].dig_h + '0';
        *(buf + 1) = uptr[ind].dig_l + '0';

        buf += 2;
        ind++;
    }

    if (buf < limit)
        *buf = 0;
    else
        *limit = 0;
}