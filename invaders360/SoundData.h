#pragma once
#include <cstdint>
#include <cstring>

struct SoundData {
    uint32_t sampleRate;
    uint32_t length;
    uint8_t* data;

    SoundData() : sampleRate(0), length(0), data(0) {}
    SoundData(const SoundData& o);
    SoundData& operator=(const SoundData& o);
    ~SoundData();
};