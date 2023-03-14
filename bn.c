#include "bn.h"
bool bn_new(bn *p, __u32 size)
{
    p->num = kmalloc(sizeof(__u32) * size, GFP_KERNEL);
    p->size = p->num ? size : 0;
    return !!p->num;
}

void bn_free(bn *p)
{
    p->size = 0;
    kfree(p->num);
}

void bn_cpy(bn *dest, bn *src) {}

static __u32 bn_clz(const bn *p)
{
    __u32 cnt = 0;
    for (__s64 i = p->size - 1; i >= 0; i--) {
        if (p->num[i]) {
            cnt += __builtin_clz(p->num[i]);
        } else {
            cnt += 32;
        }
    }
    return cnt;
}

static __u32 bn_msb(const bn *p)
{
    return (p->size << 5) - bn_clz(p);
}


static bool bn_resize(bn *p, __u32 size)
{
    if (!size)
        return false;

    if (p->size >= size)
        return true;

    size_t s = sizeof(__u32) * p->size;
    p->num = krealloc(p->num, s, GFP_KERNEL);
    if (!p->num)
        return false;
    memset(p->num + p->size, 0, s);
    p->size = size;
    return true;
}


void bn_set_zero(bn *a)
{
    memset(a->num, 0, sizeof(__u32) * a->size);
}

void bn_swap(bn *a, bn *b)
{
    bn tmp = *a;
    *a = *b;
    *b = tmp;
}

void bn_add(bn *a, bn *b, bn *c)
{
    __u32 digits = BN_MAX(bn_msb(a), bn_msb(b));
    digits = BN_DIV_ROUND(digits, 32) + !digits;
    bn_resize(c, digits);

    if (a->size < b->size) {
        bn_swap(a, b);
    }
    __u64 carry = 0;
    for (__u32 i = 0; i < b->size; i++) {
        __u64 x = a->num[i];
        __u64 y = b->num[i];
        carry += x + y;
        c->num[i] = carry;
        carry >>= 32;
    }

    for (__u32 i = b->size; i < a->size && carry; i++) {
        carry += a->num[i];
        c->num[i] = carry;
        carry >>= 32;
    }
}

void bn_lshift(bn *a, __u32 s) {}