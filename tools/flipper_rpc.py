"""Minimal Flipper Zero RPC client: screen stream and input events.

The protocol qFlipper uses: after `start_rpc_session` on the USB serial CLI,
length-prefixed protobuf messages (flipperzero-protobuf: flipper.proto,
gui.proto), encoded by hand so that pyserial is the only dependency.

Usage: flipper_rpc.py <screen.png>  (captures the current screen)
"""

import struct
import time
import zlib
from pathlib import Path

import serial

KEYS = {"up": 0, "down": 1, "right": 2, "left": 3, "ok": 4, "back": 5}
PRESS, RELEASE, SHORT = 0, 1, 2

W, H = 128, 64


def varint(n: int) -> bytes:
    out = bytearray()
    while True:
        b = n & 0x7F
        n >>= 7
        out.append(b | (0x80 if n else 0))
        if not n:
            return bytes(out)


def field_varint(num: int, value: int) -> bytes:
    return varint(num << 3) + varint(value)


def field_bytes(num: int, data: bytes) -> bytes:
    return varint(num << 3 | 2) + varint(len(data)) + data


def parse(data: bytes) -> dict[int, list]:
    fields: dict[int, list] = {}
    i = 0
    while i < len(data):
        tag, i = read_varint(data, i)
        num, kind = tag >> 3, tag & 7
        if kind == 0:
            value, i = read_varint(data, i)
        elif kind == 2:
            n, i = read_varint(data, i)
            value, i = data[i : i + n], i + n
        elif kind == 5:
            value, i = data[i : i + 4], i + 4
        elif kind == 1:
            value, i = data[i : i + 8], i + 8
        else:
            raise ValueError(f"wire type {kind}")
        fields.setdefault(num, []).append(value)
    return fields


def read_varint(data: bytes, i: int) -> tuple[int, int]:
    shift = value = 0
    while True:
        b = data[i]
        i += 1
        value |= (b & 0x7F) << shift
        if not b & 0x80:
            return value, i
        shift += 7


class Flipper:
    def __init__(self, port: str = "/dev/ttyACM0"):
        self.port = serial.Serial(port, 230400, timeout=2)
        self.command_id = 0
        self.buf = b""
        self.frame: bytes | None = None
        self.port.reset_input_buffer()
        self.port.write(b"\r")
        self._read_until(b">: ")
        self.port.write(b"start_rpc_session\r")
        # The CLI echoes the command: the binary protocol starts after it.
        self._read_until(b"start_rpc_session\r\n")

    def _read_until(self, marker: bytes) -> None:
        data = b""
        deadline = time.time() + 5
        while marker not in data:
            if time.time() > deadline:
                raise TimeoutError(f"no {marker!r} in {data[-200:]!r}")
            data += self.port.read(1)

    def send(self, num: int, payload: bytes) -> None:
        self.command_id += 1
        main = field_varint(1, self.command_id) + field_bytes(num, payload)
        self.port.write(varint(len(main)) + main)

    def poll(self, timeout: float = 0.05) -> None:
        """Reads the pending messages; keeps the last screen frame."""
        self.port.timeout = timeout
        self.buf += self.port.read(self.port.in_waiting or 1)
        while self.buf:
            try:
                n, i = read_varint(self.buf, 0)
            except IndexError:
                return
            if len(self.buf) < i + n:
                more = self.port.read(i + n - len(self.buf))
                self.buf += more
                if len(self.buf) < i + n:
                    return
            msg, self.buf = self.buf[i : i + n], self.buf[i + n :]
            fields = parse(msg)
            if 22 in fields:
                self.frame = parse(fields[22][0])[1][0]

    def start_stream(self) -> None:
        self.send(20, b"")
        deadline = time.time() + 5
        while self.frame is None:
            if time.time() > deadline:
                raise TimeoutError("no screen frame")
            self.poll()

    def wait(self, seconds: float) -> None:
        end = time.time() + seconds
        while time.time() < end:
            self.poll()

    def key(self, name: str, kinds=(PRESS, SHORT, RELEASE)) -> None:
        for kind in kinds:
            self.send(23, field_varint(1, KEYS[name]) + field_varint(2, kind))

    def pixels(self) -> list[list[bool]]:
        """Frame buffer: 8 pages of 128 columns, one byte = 8 rows (LSB on top)."""
        f = self.frame
        assert f is not None, "start_stream() first"
        return [[bool(f[(y // 8) * W + x] >> (y % 8) & 1) for x in range(W)] for y in range(H)]

    def close(self) -> None:
        self.send(21, b"")
        self.port.close()


def write_png(path: Path, pixels: list[list[bool]], scale: int = 4) -> None:
    """Writes the screen as qFlipper shows it: black on the orange LCD."""
    on, off = b"\x00\x00\x00", b"\xff\x82\x00"
    raw = bytearray()
    for row in pixels:
        line = b"".join((on if p else off) * scale for p in row)
        for _ in range(scale):
            raw += b"\x00" + line

    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    header = struct.pack(">IIBBBBB", W * scale, H * scale, 8, 2, 0, 0, 0)
    path.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", header)
        + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        + chunk(b"IEND", b"")
    )


if __name__ == "__main__":
    import sys

    flipper = Flipper()
    flipper.start_stream()
    flipper.wait(0.5)
    write_png(Path(sys.argv[1]), flipper.pixels())
    flipper.close()
