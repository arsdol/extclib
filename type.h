#pragma once

#include <stdint.h>

typedef enum {
    DECIMAL_ELEM,
    REAL_ELEM,
    STRING_ELEM,
} vtype_t;

typedef union value_t {
    int32_t decimal;
    double real;
    uint8_t *string;
} value_t;

extern void *decimal(int32_t x);
extern void *string(uint8_t *x);
extern void *real(double x);
