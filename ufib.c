#include <stdio.h>
#include <time.h>
#include "bn.h"
#define NANOSECOND(x) ((x).tv_sec * 1e9 + (x).tv_nsec)
char *bn_to_string(const bn *src)
{
    // log10(x) = log2(x) / log2(10) ~= log2(x) / 3.322
    size_t len = (8 * sizeof(__u32) * src->size) / 3 + 2 + 1;
    char *s = kmalloc(len, GFP_KERNEL);
    char *p = s;

    memset(s, '0', len - 1);
    s[len - 1] = '\0';

    /* src.number[0] contains least significant bits */
    for (int i = src->size - 1; i >= 0; i--) {
        /* walk through every bit of bn */
        for (unsigned int d = 1U << 31; d; d >>= 1) {
            /* binary -> decimal string */
            int carry = !!(d & src->num[i]);
            for (int j = len - 2; j >= 0; j--) {
                s[j] += s[j] - '0' + carry;
                carry = (s[j] > '9');
                if (carry)
                    s[j] -= 10;
            }
        }
    }
    // skip leading zero
    while (p[0] == '0' && p[1] != '\0') {
        p++;
    }

    memmove(s, p, strlen(p) + 1);
    return s;
}

void bn_print(const bn *src)
{
    char *sp = bn_to_string(src);
    printf("%s\n", sp);
    free(sp);
}

static void test_bn_fast_doubling(__u64 k)
{
    __u64 mask = 1UL << 63;
    mask >>= __builtin_clzl(k);
    bn a, b, c, d;
    bn_new(&a, 1);
    bn_new(&b, 1);
    bn_new(&c, 1);
    bn_new(&d, 1);
    if (!a.num || !b.num || !c.num || !d.num)
        goto tail;
    bn_set_zero(&a);
    bn_set_zero(&b);
    bn_set_zero(&c);
    bn_set_zero(&d);

    a.num[0] = 0;
    b.num[0] = 1;

    for (; mask; mask >>= 1) {
        bn_cpy(&d, &b);
        bn_lshift(&d, 1);
        bn_diff(&d, &a, &d);
        bn_mult(&a, &d, &c);

        bn_mult(&a, &a, &d);
        bn_mult(&b, &b, &a);
        bn_add(&d, &a, &d);

        if (mask & k) {
            bn_add(&c, &d, &b);
            bn_swap(&a, &d);
        } else {
            bn_swap(&c, &a);
            bn_swap(&d, &b);
        }
    }
tail:
    bn_free(&a);
    bn_free(&b);
    bn_free(&c);
    bn_free(&d);
}

static void test_bn_cmp()
{
    bn a, b;
    bn_new(&a, 5);
    bn_new(&b, 5);
    if (!a.num)
        goto tail;

    bn_set_zero(&a);
    bn_set_zero(&b);

    a.num[0] = 2;
    a.num[1] = 1;
    a.num[2] = 1;
    a.num[3] = 1023;

    b.num[0] = 1;
    b.num[1] = 1;
    b.num[2] = 1;
    b.num[3] = 1023;
    int c = bn_cmp(&a, &b);
    int d = bn_cmp(&b, &a);
    printf("Compare: %d\n", c);
    printf("Compare: %d\n", d);

tail:
    bn_free(&a);
    bn_free(&b);
}


static void test_bn_cpy()
{
    bn a, b;
    bn_new(&a, 2);
    bn_new(&b, 2);
    if (!a.num)
        goto tail;

    bn_cpy(&a, &b);
    bn_set_zero(&a);
    bn_set_zero(&b);

tail:
    bn_free(&a);
    bn_free(&b);
}

static void test_bn_diff(__u64 k)
{
    bn a, b, c;
    bn_new(&a, 1);
    bn_new(&b, 1);
    bn_new(&c, 1);
    if (!a.num)
        goto tail;
    bn_set_zero(&a);
    bn_set_zero(&b);
    bn_set_zero(&c);
    a.num[0] = 0;
    b.num[0] = 1;

    for (int i = 0; i < k; i++) {
        bn_add(&a, &b, &c);
        if (!c.num)
            goto tail;
        bn_swap(&a, &b);
        bn_swap(&c, &b);
    }

    bn_diff(&a, &b, &c);
    bn_print(&c);
tail:
    bn_free(&a);
    bn_free(&b);
    bn_free(&c);
}


static void test_bn_lshift()
{
    bn a;
    bn_new(&a, 2);
    if (!a.num)
        goto tail;

    bn_set_zero(&a);
    a.num[0] = -1;
    a.num[1] = -1;

    {
        char *ap = bn_to_string(&a);
        printf("a: %s\n", ap);
        free(ap);
    }
    bn_lshift(&a, 10);
    {
        char *ap = bn_to_string(&a);
        printf("a: %s\n", ap);
        free(ap);
    }

tail:
    bn_free(&a);
}

static void test_bn_add(long long k)
{
    bn a, b, c;
    bn_new(&a, 1);
    bn_new(&b, 1);
    bn_new(&c, 1);
    if (!a.num || !b.num || !c.num)
        goto tail;

    bn_set_zero(&a);
    bn_set_zero(&b);
    bn_set_zero(&c);
    a.num[0] = 0;
    b.num[0] = 1;

    for (int i = 0; i < k; i++) {
        bn_add(&a, &b, &c);
        if (!c.num)
            goto tail;
        bn_swap(&a, &b);
        bn_swap(&c, &b);
    }

    bn_print(&a);
tail:
    bn_free(&a);
    bn_free(&b);
    bn_free(&c);
}

static void test_bn_mult()
{
    bn a, b, c;
    bn_new(&a, 2);
    bn_new(&b, 2);
    bn_new(&c, 2);
    if (!a.num || !b.num || !c.num)
        goto tail;
    bn_set_zero(&a);
    bn_set_zero(&b);
    bn_set_zero(&c);

    unsigned long f89 = 1779979416004714189;
    unsigned long f90 = 2880067194370816120;
    a.num[0] = f89 & 0xFFFFFFFF;
    a.num[1] = f89 >> 32;
    b.num[0] = f90 & 0xFFFFFFFF;
    b.num[1] = f90 >> 32;
    bn_mult(&a, &b, &c);

    bn_print(&c);
tail:
    bn_free(&a);
    bn_free(&b);
    bn_free(&c);
}

int main()
{
    while (0) {
        test_bn_cmp();
        test_bn_diff(95);
        test_bn_add(94);
        test_bn_cpy();
        test_bn_lshift();
        test_bn_add(92);
        test_bn_mult();
        test_bn_fast_doubling(93);
    };

    struct timespec t1, t2;
    for (int i = 0; i < 500; i++) {
        clock_gettime(CLOCK_MONOTONIC, &t1);
        test_bn_fast_doubling(i);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        long long ut = (long long) (NANOSECOND(t2) - NANOSECOND(t1));
        printf("%d %lld\n", i, ut);
    }
    return 0;
}