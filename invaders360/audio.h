#pragma once
#include <xaudio2.h>
#include <stdint.h>
#include "SoundData.h"

class audio {
public:
    audio() : xAudio(nullptr), master(nullptr) {}
    ~audio();
    HRESULT init();
    HRESULT createVoice(IXAudio2SourceVoice*& voice, WAVEFORMATEX& wfx);
private:
    audio(const audio&);
    audio& operator=(const audio&);
    IXAudio2* xAudio;
    IXAudio2MasteringVoice* master;
};