#include "stdafx.h"
#include "audio.h"

HRESULT audio::init()
{
    HRESULT hr = XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return hr;
    return xAudio->CreateMasteringVoice(&master);
}

void audio::play(uint8_t* data, size_t length) {
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = 1;
    wfx.nSamplesPerSec  = 11025;
    wfx.wBitsPerSample  = 8;
    wfx.nBlockAlign     = 1;
    wfx.nAvgBytesPerSec = 11025;

   xAudio->CreateSourceVoice(&source, &wfx);
   XAUDIO2_BUFFER buf = {0};
    buf.pAudioData = data;
    buf.AudioBytes = 3263;
    buf.LoopCount = XAUDIO2_LOOP_INFINITE;
    source->SubmitSourceBuffer(&buf);
    source->Start(0);
}
