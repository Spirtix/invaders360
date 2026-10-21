#include <xaudio2.h>
#include <stdint.h>

class audio {
public:
    HRESULT init();
    void play(uint8_t* data, size_t length);
private:
    IXAudio2* xAudio;
    IXAudio2MasteringVoice* master;
    IXAudio2SourceVoice* source;
};