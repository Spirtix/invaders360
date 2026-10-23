#include "stdafx.h"
#include "audio.h"

HRESULT audio::init()
{
    HRESULT hr = XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return hr;
    return xAudio->CreateMasteringVoice(&master);
}

void audio::play(SoundData& data) {
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = 1;
    wfx.nSamplesPerSec  = data.sampleRate;
    wfx.wBitsPerSample  = 8;
    wfx.nBlockAlign     = 1;
    wfx.nAvgBytesPerSec = data.sampleRate;

    xAudio->CreateSourceVoice(&source, &wfx);
    XAUDIO2_BUFFER buf = {0};
    buf.pAudioData = data.data;
    buf.AudioBytes = data.length;
    buf.LoopCount = XAUDIO2_LOOP_INFINITE;
    source->SubmitSourceBuffer(&buf);
    source->Start(0);
}
