#ifndef BIGNUM_H
#define BIGNUM_H

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <stdbool.h>

#include "xs.h"

#define bn_empty() ((bn_t){.space_left = 0});

typedef xs bn_t;

bn_t *bn_new(bn_t *bn, const char *p);
bn_t *bn_add(const bn_t *a, const bn_t *b, bn_t *res);
size_t bn_space_for_buf(const bn_t *bn);
void bn_to_digit_string(const bn_t *bn, char *buf, size_t len);

#endif