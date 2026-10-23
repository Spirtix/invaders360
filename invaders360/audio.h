#include <xaudio2.h>
#include <stdint.h>
#include "SoundData.h"

class audio {
public:
    HRESULT init();
    void play(SoundData& data);
private:
    IXAudio2* xAudio;
    IXAudio2MasteringVoice* master;
    IXAudio2SourceVoice* source;
};