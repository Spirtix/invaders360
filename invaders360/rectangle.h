#pragma once
#include <stdint.h>

struct rectangle {
    uint32_t x, y, width, height, color;
    rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t c)
        : x(x), y(y), width(w), height(h), color(c) {}
};