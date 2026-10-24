#include "stdafx.h"
#include "sound.h"
#include <cstring>
#include <stdexcept>

sound::sound(audio& audio, SoundData& soundData) : data(soundData.data) {
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = 1;
    wfx.nSamplesPerSec  = soundData.sampleRate;
    wfx.wBitsPerSample  = 8;
    wfx.nBlockAlign     = 1;
    wfx.nAvgBytesPerSec = soundData.sampleRate;

    if (FAILED(audio.createVoice(voice, wfx)))
        throw std::runtime_error("Sound: CreateVoice failed");

    buffer.pAudioData = data.get();
    buffer.AudioBytes = soundData.length;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = 0;
    buffer.LoopBegin  = 0;
    buffer.LoopLength = soundData.length;
}

sound::sound(sound&& o) : data(std::move(o.data)), buffer(o.buffer), voice(o.voice) {
    o.voice = nullptr;
    o.buffer = XAUDIO2_BUFFER();
}

sound::~sound() {
    if (voice) voice->DestroyVoice();
}

void sound::play() {
    stop();
    voice->SubmitSourceBuffer(&buffer);
    voice->Start();
}

void sound::stop() {
    voice->Stop();
    voice->FlushSourceBuffers();
}

void sound::setLoop(bool loop) {
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
}
