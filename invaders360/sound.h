#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <xaudio2.h>
#include "SoundData.h"
#include "audio.h"

class sound {
public:
    sound(audio& audio, SoundData& soundData);
    sound(sound&& o);
    ~sound();
    void play();
    void stop();
private:
    sound(const sound&);
    sound& operator=(const sound&);
    sound& operator=(sound&&);
    uint32_t sampleRate;
    uint32_t length;
    std::unique_ptr<uint8_t[]> data;
    XAUDIO2_BUFFER buffer;
    IXAudio2SourceVoice* voice;
};
