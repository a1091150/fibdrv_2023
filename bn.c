#include "bn.h"
static __u32 bn_clz(const bn *p)
{
    __u32 cnt = 0;
    for (__s64 i = p->size - 1; i >= 0; i--) {
        if (p->num[i]) {
            cnt += __builtin_clz(p->num[i]);
            break;
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

    p->num = krealloc(p->num, sizeof(__u32) * size, GFP_KERNEL);
    if (!p->num)
        return false;
    memset(p->num + p->size, 0, sizeof(__u32) * (size - p->size));
    p->size = size;
    return true;
}

int bn_cmp(bn *a, bn *b)
{
    __u32 abits = bn_msb(a);
    __u32 bbits = bn_msb(b);

    if (abits == bbits) {
        __u32 i = abits >> 5;
        while (i > 0 && a->num[i] == b->num[i]) {
            i--;
        }

        if (a->num[i] == b->num[i])
            return 0;
        return ((a->num[i] < b->num[i]) << 1) - 1;
    }
    return ((abits < bbits) << 1) - 1;
}

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



void bn_cpy(bn *dest, bn *src)
{
    if (dest->size > src->size) {
        memcpy(dest->num, src->num, sizeof(__u32) * src->size);
        memset(dest->num + src->size, 0, dest->size - src->size);
        return;
    }
    bn_resize(dest, src->size);
    memcpy(dest->num, src->num, sizeof(__u32) * src->size);
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
    __u32 digits = BN_MAX(bn_msb(a), bn_msb(b)) + 1;
    digits = BN_DIV_ROUND(digits, 32) + !digits;
    bn_resize(c, digits);

    bn aa = *a;
    bn bb = *b;
    if (a->size < b->size)
        bn_swap(&aa, &bb);

    __u64 carry = 0;
    __u32 i = 0;
    for (; i < b->size; i++) {
        carry += (__u64) aa.num[i] + (__u64) bb.num[i];
        c->num[i] = carry;
        carry >>= 32;
    }

    for (; i < a->size; i++) {
        carry += a->num[i];
        c->num[i] = carry;
        carry >>= 32;
    }

    if (carry) {
        c->num[i] = carry;
    }

    if (isSwapped) {
        bn_swap(a, b);
    }
}

void bn_diff(bn *a, bn *b, bn *c)
{
    int abcmp = bn_cmp(a, b);
    if (abcmp == 0) {
        bn_set_zero(c);
        return;
    }
    bn aa = *a;
    bn bb = *b;

    if (abcmp == 1)
        bn_swap(&aa, &bb);

    bn_resize(c, a->size);

    __u64 carry = 0;
    __u32 i = 0;

    for (; i < bb.size; i++) {
        __u64 x = (__u64) aa.num[i] - (__u64) bb.num[i] - carry;
        __u64 sign = !!(x >> 32);
        c->num[i] = x + (sign << 32);
        carry = sign;
    }

    for (; i < aa.size; i++) {
        __u64 x = (__u64) aa.num[i] - carry;
        __u64 sign = !!(x >> 32);
        c->num[i] = x + (sign << 32);
        carry = sign;
    }

    if (carry) {
        c->num[i] -= carry;
    }

    if (abcmp == 1)
        bn_swap(a, b);
}

void bn_mult(bn *a, bn *b, bn *c)
{
    __u32 digits = bn_msb(a) + bn_msb(b);
    digits = BN_DIV_ROUND(digits, 32) + !digits;
    bn_resize(c, digits);
    if (!c->num)
        return;

    bn_set_zero(c);
    for (__u32 i = 0; i < a->size; i++) {
        for (__u32 j = 0; j < b->size; j++) {
            __u64 carry = (__u64) a->num[i] * (__u64) b->num[j];
            __u32 k = i + j;
            do {
                __u64 s = carry + c->num[k];
                c->num[k] = s;
                carry = s >> 32;
                k++;
            } while (k < c->size && carry);
        }
    }
}
/* Maximum of left shift size less than 32*/
void bn_lshift(bn *a, __u32 s)
{
    s = s & 0b11111;
    __u64 carry = 0;
    __u32 i = 0;
    for (; i < a->size; i++) {
        __u64 x = a->num[i];
        a->num[i] = (x << s) | carry;
        carry = x >> (32 - s);
    }

    if (carry) {
        bn_resize(a, i + 1);
        a->num[i] = carry;
    }
}