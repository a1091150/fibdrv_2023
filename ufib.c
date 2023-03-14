#include <stdio.h>
#include "bn.h"
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

static void test_bn_cpy()
{
    bn a, b, c;
    bn_new(&a, 2);
    bn_new(&b, 2);
    bn_new(&c, 2);
    if (!a.num || !b.num || !c.num)
        goto tail;
    bn_cpy(&a, &b);
    bn_diff(&a, &b, &c);
    bn_lshift(&a, &b);
tail:
    bn_free(&a);
    bn_free(&b);
    bn_free(&c);
}
static void test_bn_add(long long k)
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
    a.num[0] = 0;
    b.num[0] = 1;

    for (int i = 0; i < k; i++) {
        bn_add(&a, &b, &c);
        if (!c.num)
            goto tail;
        bn_swap(&a, &b);
        bn_swap(&c, &b);
        /* It does not need to set zero in most of the time. */
        bn_set_zero(&c);
    }

    char *ap = bn_to_string(&a);
    char *bp = bn_to_string(&b);
    printf("%lld a: %s\n", k, ap);
    printf("%lld b: %s\n\n", k + 1, bp);
    free(ap);
    free(bp);

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

    char *cp = bn_to_string(&c);
    printf("89 * 90 c: %s\n\n", cp);
    free(cp);
tail:
    bn_free(&a);
    bn_free(&b);
    bn_free(&c);
}

int main()
{
    test_bn_cpy();
    test_bn_add(91);
    test_bn_add(92);
    test_bn_mult();
    return 0;
}