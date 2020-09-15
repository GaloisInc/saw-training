#include <stdint.h>

void swap(uint32_t *x, uint32_t *y) {
    uint32_t tmp = *x;
    *x = *y;
    *y = tmp;
}

// BEGIN ROTR3
void rotr3(uint32_t *x, uint32_t *y, uint32_t *z) {
    uint32_t tmp = *x;
    *x = *z;
    *y = *x;
    *z = tmp;
}
// END ROTR3

// BEGIN ROTR3_FIXED
void rotr3_fixed(uint32_t *x, uint32_t *y, uint32_t *z) {
    uint32_t tmp_x = *x;
    uint32_t tmp_y = *y;
    *x = *z;
    *y = tmp_x;
    *z = tmp_y;
}
// END ROTR3_FIXED
