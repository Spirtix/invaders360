#include "stdafx.h"
#include "audio.h"

audio::~audio() {
    if (master) master->DestroyVoice();
    if (xAudio) xAudio->Release();
}

HRESULT audio::init()
{
    HRESULT hr = XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return hr;
    return xAudio->CreateMasteringVoice(&master);
}

HRESULT audio::createVoice(IXAudio2SourceVoice*& voice, WAVEFORMATEX& wfx) {
    return xAudio->CreateSourceVoice(&voice, &wfx);
}
