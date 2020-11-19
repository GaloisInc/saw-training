#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// BEGIN POP_COUNT
/*
 * Returns a count of the set bits in a word.
 * From Henry S. Warren Jr.'s Hacker's Delight
 */
int pop_count(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}
// END POP_COUNT

// BEGIN POP_CHECK
/* Test pop_count on a few values to make sure it's at least sometimes correct */
bool pop_check(){
    return (pop_count(0x0) == 0)
         &&(pop_count(0x3) == 2)
         &&(pop_count(0xFFFFFFFF) == 32)
         &&(pop_count(0xAAAAAAAA) == 16)
         &&(pop_count(0x55555555) == 16);
}
// END POP_CHECK

// BEGIN POP_SPEC
/*
 * Slow and hopefully correct population count function.
 */
int pop_spec(uint32_t x) {
    uint32_t pop = 0;
    uint32_t mask = 1;
    for (int i = 0; i < 32; i++) {
        if (x & mask) { pop++; }
        mask = mask << 1;
    }
    return pop;
}

/*
 * Check the optimized pop_count function against our "believed
 * correct" version. Returns TRUE if they agree and FALSE if they disagree
 */
bool pop_spec_check(uint32_t x) {
    return (pop_spec(x) == pop_count(x));
}
// END POP_SPEC

// BEGIN POP_MUL
/* A version of popcount that uses multiplication */
int pop_count_mul(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = ((x + (x >> 4)) & 0x0F0F0F0F);
    return (x * 0x01010101) >> 24;
}
// END POP_MUL

// BEGIN POP_SPARSE
/* A version of popcount that uses an indefinite while loop(!) */
int pop_count_sparse(uint32_t x) {
    int n;
    n = 0;
    while (x != 0) {
        n = n + 1;
        x = x & (x - 1);
    }
    return n;
}
// END POP_SPARSE

// BEGIN POP_BROKEN1
int pop_count_broken1(uint32_t x) {
    int r = pop_count(x);
    if ((r == 21) && ((x & 0xF000000F) == 0)) r++;
    return r;
}

bool pop_broken1_check(uint32_t x) {
    return (pop_spec(x) == pop_count_broken1(x));
}
// END POP_BROKEN1

// BEGIN POP_BROKEN2
int pop_count_broken2(uint32_t x) {
    if (x == 0xDEADBEEF) return 22;
    return pop_count(x);
}

bool pop_broken2_check(uint32_t x) {
    return (pop_spec(x) == pop_count_broken2(x));
}
// END POP_BROKEN2

// BEGIN POP_RANDOM_VALUE_TEST
void random_value_test(int (*fun)(uint32_t), char *name) {
    srand(time(NULL));

    int failures = 0;
    for (int i = 0; i < 100000; i ++) {
        uint32_t x = rand();
        int test = (*fun)(x);
        int check = pop_spec(x);
        if (test != check) {
            printf("Test failure: %s(%u) was %u, != %u\n",
                    name, x, test, check);
            failures++;
        }
    }
    if (failures == 0) {
        printf("Testing %s succeeded!\n", name);
    }
}
// END POP_RANDOM_VALUE_TEST

int main() {
    random_value_test(&pop_count, "pop_count");
    random_value_test(&pop_count_mul, "pop_count_mul");
    random_value_test(&pop_count_sparse, "pop_count_sparse");
    random_value_test(&pop_count_broken1, "pop_count_broken1");
}
