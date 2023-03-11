#ifndef _BN_H_
#define _BN_H_
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/types.h>
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

void bn_swap(bn *a, bn *b);

void bn_add(bn *a, bn *b, bn *c);
void bn_set_zero(bn *p);
#endif /* !_BN_H_*/