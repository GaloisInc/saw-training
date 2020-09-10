#include <assert.h>
#include <stdint.h>
#include <stdio.h>
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

// BEGIN SWAP_SPEC
bool general_swap_spec(void (*fun)(uint32_t *, uint32_t *), uint32_t a, uint32_t b) {
    uint32_t x = a, y = b;
    (*fun)(&x, &y);
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

void test_swap_function(void (*fun)(uint32_t *, uint32_t *), char *descr, uint32_t x, uint32_t y) {
    printf("[%s] Testing with %u and %u... ", descr, x, y);
    if (general_swap_spec(fun, x, y)) {
        printf("OK\n");
    } else {
        printf("FAILED\n");
    }
}

void test_swap(char *descr, uint32_t x, uint32_t y) {
    test_swap_function(&swap, descr, x, y);
}

// BEGIN SWAP_CHOSEN_VALUE_TEST
void chosen_value_test(void (*fun)(uint32_t *, uint32_t *)) {
    test_swap_function(fun, "Chosen", 1, 2);
    test_swap_function(fun, "Chosen", 2429, 98423);
    test_swap_function(fun, "Chosen", 8347853, 0);
    test_swap_function(fun, "Chosen", 5, 5);
    // ...
}
// END SWAP_CHOSEN_VALUE_TEST

// BEGIN SWAP_RANDOM_VALUE_TEST
void random_value_test(void (*fun)(uint32_t *, uint32_t *)) {
    srand(time(NULL));

    for (int i = 0; i < 100; i ++) {
        test_swap_function(fun, "Random", rand(), rand());
    }
}
// END SWAP_RANDOM_VALUE_TEST

int main() {
    printf("Beginning chosen-value tests for swap\n");
    chosen_value_test(&swap);
    printf("\n");
    printf("Ending chosen-value tests for swap\n");

    printf("Beginning random tests for swap\n");
    random_value_test(&swap);
    printf("Ending random tests for swap\n");


}
