#!/usr/bin/env python3
"""PPM (P6) -> PNG, тек стандартты кітапханамен."""
import struct
import sys
import zlib


def main(src, dst):
    with open(src, "rb") as f:
        assert f.readline().strip() == b"P6"
        w, h = map(int, f.readline().split())
        f.readline()
        data = f.read()
    raw = b"".join(b"\x00" + data[y*w*3:(y+1)*w*3] for y in range(h))

    def chunk(t, d):
        c = struct.pack(">I", len(d)) + t + d
        return c + struct.pack(">I", zlib.crc32(t + d) & 0xffffffff)

    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)) \
        + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    with open(dst, "wb") as f:
        f.write(png)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
