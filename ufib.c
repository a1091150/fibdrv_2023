#include <stdio.h>
#include "bn.h"
static void test_bn_add(long long k)
{
    bn a, b, c;
    bn_new(&a, 2);
    bn_new(&b, 2);
    bn_new(&c, 2);
    if (!a.num || !b.num || !c.num)
        goto tail;

    for (int i = 0; i < k; i++) {
        bn_add(&a, &b, &c);
        if (!c.num)
            goto tail;
        bn_swap(&a, &b);
        bn_swap(&c, &b);
        /* It does not need to set zero in most of the time. */
        bn_set_zero(&c);
    }

tail:
    bn_free(&a);
    bn_free(&b);
    bn_free(&c);
    return 0;
}

static void test_bn_sub() {}

int main()
{
    test_bn_add(93);
    test_bn_sub();
    printf("Hello World!\n");
    return 0;
}