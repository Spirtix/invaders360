#include "stdafx.h"
#include "loader.h"

struct FileCloser {
    void operator()(FILE* f) const { if (f) fclose(f); }
};
typedef std::unique_ptr<FILE, FileCloser> FilePtr;

static bool readU32(FILE* f, uint32_t& out) {
    return fread(&out, sizeof(out), 1, f) == 1;
}

std::vector<SoundData> loader::LoadSoundPak(const char* path) {
    FilePtr f(fopen(path, "rb"));
    if (!f)
        throw std::runtime_error("SoundPak: cannot open pak file");

    uint32_t count = 0;
    if (!readU32(f.get(), count))
        throw std::runtime_error("SoundPak: unexpected end of file");

    std::vector<SoundData> soundData(count);

    for (uint32_t i = 0; i < count; ++i) {
        SoundData& s = soundData[i];

        if (!readU32(f.get(), s.sampleRate) || !readU32(f.get(), s.length))
            throw std::runtime_error("SoundPak: unexpected end of file");

        if (s.length > 0) {
            s.data = new uint8_t[s.length];
            if (fread(s.data, 1, s.length, f.get()) != s.length)
                throw std::runtime_error("SoundPak: unexpected end of file");
        }
    }

    return soundData;
}
