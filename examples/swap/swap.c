#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// BEGIN SWAP
void swap(uint32_t *x, uint32_t *y) {
    uint32_t tmp = *x;
    *x = *y;
    *y = tmp;
}
// END SWAP

// BEGIN SWAP_SPEC
bool swap_spec(uint32_t a, uint32_t b) {
    uint32_t x = a, y = b;
    swap(&x, &y);
    return x == b && y == a;
}
// END SWAP_SPEC

// BEGIN SWAP_BROKEN1
void swap_broken1(uint32_t *x, uint32_t *y) {
    uint32_t tmp = *x;
    *x = tmp;
    *y = *y;
}
// END SWAP_BROKEN1

// BEGIN SWAP_BROKEN2
void swap_broken2(uint32_t *x, uint32_t *y) {
    uint32_t tmp = *x;
    if (*x != 4142351) {
        *x = *y;
        *y = tmp;
    }
}
// END SWAP_BROKEN2

// BEGIN SWAP_BROKEN3
void swap_broken3(uint32_t *x, uint32_t *y) {
    uint32_t tmp = *x;
    if (*x == (*y << 5)) {
        y = NULL;
    }
    *x = *y;
    *y = tmp;
}
// END SWAP_BROKEN3


// BEGIN SWAP_CHOSEN_VALUE_TEST
void chosen_value_test(void (*fun)(uint32_t *, uint32_t *)) {
    uint32_t x = 1, y = 2;
    (*fun)(&x, &y);
    assert(x == 2 && y == 1);

    x = 2429;
    y = 8563;
    (*fun)(&x, &y);
    assert(x == 8563 && y == 2429);

    // ...
}
// END SWAP_CHOSEN_VALUE_TEST

// BEGIN SWAP_RANDOM_VALUE_TEST
void random_value_test(void (*fun)(uint32_t *, uint32_t *)) {
    srand(time(NULL));

    for (int i = 0; i < 1000; i ++) {
        uint32_t x = rand(), y = rand();
        uint32_t old_x = x, old_y = y;
        (*fun)(&x, &y);
        assert(x == old_y && y == old_x);
    }
}
// END SWAP_RANDOM_VALUE_TEST
