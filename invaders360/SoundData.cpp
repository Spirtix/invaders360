#include "stdafx.h"
#include "SoundData.h"
#include <cstring>

SoundData::SoundData(const SoundData& o) : sampleRate(o.sampleRate), length(o.length), data(0) {
    if (length) {
        data = new uint8_t[length];
        memcpy(data, o.data, length);
    }
}

SoundData& SoundData::operator=(const SoundData& o) {
    if (this != &o)
    {
        uint8_t* copy = 0;
        if (o.length)
        {
            copy = new uint8_t[o.length];
            memcpy(copy, o.data, o.length);
        }
        delete[] data;
        data = copy;
        sampleRate = o.sampleRate;
        length = o.length;
    }
    return *this;
}

SoundData::~SoundData() {
    delete[] data;
}