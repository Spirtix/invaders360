#!/usr/bin/env python3
"""
Pack 0.wav .. 8.wav into a single sounds.pak file.
The input wave files must be 8 bit PCM.
The packer expects little endian metadata which it writes as big endian to the resulting .pak file.

Format:
    num_files
    then, for each file:
        sample_rate
        length        (size of the data in bytes)
        data          (PCM bytes)

Usage:
    python sound_packer.py [input_dir] [output_file]
Defaults: input_dir = current directory, output_file = sounds.pak
"""

import struct
import sys
from pathlib import Path

NUM_FILES = 9  # 0.wav .. 8.wav


def read_wav(path):
    raw = Path(path).read_bytes()

    if raw[0:4] != b"RIFF" or raw[8:12] != b"WAVE":
        raise ValueError(f"{path}: not a RIFF/WAVE file")

    sample_rate = None
    data = None
    pos = 12
    while pos + 8 <= len(raw):
        chunk_id = raw[pos:pos + 4]
        chunk_size = struct.unpack_from("<I", raw, pos + 4)[0]
        body = pos + 8

        if chunk_id == b"fmt ":
            _fmt, channels, sample_rate, _br, _ba, bits = struct.unpack_from(
                "<HHIIHH", raw, body
            )
            if bits != 8 or channels != 1:
                print(
                    f"warning: {path}: {channels} ch, {bits}-bit "
                    f"(data is copied as-is, not byte-swapped)"
                )
        elif chunk_id == b"data":
            data = raw[body:body + chunk_size]
            break

        pos = body + chunk_size + (chunk_size & 1)

    if sample_rate is None or data is None:
        raise ValueError(f"{path}: missing fmt or data chunk")

    return sample_rate, data


def main():
    in_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
    out_path = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("sounds.pak")

    with open(out_path, "wb") as out:
        out.write(struct.pack(">I", NUM_FILES))
        for i in range(NUM_FILES):
            wav = in_dir / f"{i}.wav"
            rate, data = read_wav(wav)
            out.write(struct.pack(">II", rate, len(data)))
            out.write(data)
            print(f"{wav.name}: rate={rate} length={len(data)}")

    print(f"wrote {out_path} ({out_path.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
