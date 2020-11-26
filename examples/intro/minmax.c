#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

// BEGIN MINMAX
int8_t minmax(int64_t *x, int64_t *y) {
    if (*x > *y) {
        int64_t tmp = *x;
        *x = *y;
        *y = tmp;
        return 1;
    }
    else {
        return -(*x != *y);
    }
}
// END MINMAX

int8_t minmax_xor(int64_t *x, int64_t *y) {
    if (*x > *y) {
        *x = *x ^ *y;
        *y = *y ^ *x;
        *x = *x ^ *y;
        return 1;
    } else {
        return -(*x != *y);
    }
}


// BEGIN MINMAX_TERNARY
int8_t minmax_ternary(int64_t *x, int64_t *y) {
    int64_t xv = *x, yv = *y;
    *x = xv < yv ? xv : yv;
    *y = xv < yv ? yv : xv;
    return xv < yv ? -1 : xv == yv ? 0 : 1;
}
// END MINMAX_TERNARY

int main() {
    for (int64_t i = -5; i < 5; i++) {
        for (int64_t j = -5; j < 5; j++) {
            int64_t x = i, y = j;
            int out = minmax(&x, &y);
            printf("%" PRId64 " %" PRId64 " --> %" PRId64 " %" PRId64 " (%i)\n", i, j, x, y, out);
        }
    }
}
