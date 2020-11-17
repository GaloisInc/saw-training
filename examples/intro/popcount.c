#include <stdbool.h>

// BEGIN POP_COUNT
/*
 * Returns a count of the set bits in a word.
 * From Henry S. Warren Jr.'s Hacker's Delight
 */
int pop_count(unsigned x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}
// END POP_COUNT

// BEGIN POP_SPEC
/*
 * Checks the optimized pop_count function against a very naive
 * (and slow) computation of the same value. Returns TRUE if they
 * agree and FALSE if they disagree
 */
bool pop_spec(unsigned x) {
    unsigned p = 0;
    unsigned m = 1;
    // p = population count(x) in a very simple way
    for (int i = 0; i < 32; i++) {
        if (x & m) { p++; }
        m = m << 1;
    }
    // check whether our simple p agrees with fancy pop_count(x)
    return (p == pop_count(x));
}
// END POP_SPEC

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

// BEGIN POP_MUL
/* A version of popcount that uses multiplication */
int pop_count_mul(unsigned x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = ((x + (x >> 4)) & 0x0F0F0F0F);
    return (x * 0x01010101) >> 24;
}
// END POP_MUL

// BEGIN POP_SPARSE
/* A version of popcount that uses an indefinite while loop(!) */
int pop_count_sparse(unsigned x) {
   int n;
   n = 0;
   while (x != 0) {
      n = n + 1;
      x = x & (x - 1);
   }
   return n;
}
// END POP_SPARSE
