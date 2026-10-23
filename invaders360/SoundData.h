#pragma once
#include <stdint.h>

struct SoundData {
    uint32_t sampleRate;
    uint32_t length;
    uint8_t* data;
};