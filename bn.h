#ifndef _BN_H_
#define _BN_H_
#ifndef _USERSPACEFIB
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/types.h>
#else
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define kmalloc(size, flags) (malloc(size))
#define krealloc(ptr, size, flags) (realloc((ptr), (size)))
#define kfree(p) (free(p))
typedef unsigned int __u32;
typedef signed int __s32;
typedef unsigned long __u64;
typedef signed long __s64;
#define GFP_KERNEL (0)
#endif

typedef struct _bn {
    __u32 size;
    __u32 alloc;
    __u32 *num;
} bn;
#define BN_DIV_ROUND(x, len) (((x) + (len) -1) / (len))
#define BN_ROUND_UP(x, align) BN_DIV_ROUND(x, align) * (align)
#define BN_MAX(a, b) (a) > (b) ? (a) : (b)
bool bn_new(bn *p, __u32 size);
void bn_free(bn *p);
void bn_cpy(bn *dest, bn *src);

void bn_swap(bn *a, bn *b);

/*
    If a < b, return 1.
    If a = b, return 0.
    If a > b, return -1;
*/
int bn_cmp(bn *a, bn *b);

void bn_add(bn *a, bn *b, bn *c);
void bn_diff(bn *a, bn *b, bn *c);
void bn_lshift(bn *a, __u32 s);
void bn_mult(bn *a, bn *b, bn *c);
void bn_set_zero(bn *a);
#endif /* !_BN_H_*/