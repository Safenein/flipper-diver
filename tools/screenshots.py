"""Plays a round of Flipper Diver over RPC and captures each screen.

Start the app on the title screen first (`launch`). The arrows of the current
sequence are read from the screen, by matching the templates that view.c
draws, and typed back. The README shows the captures of docs/screenshots/.

Usage: screenshots.py [output dir, default docs/screenshots]
"""

import sys
import time
from pathlib import Path

from flipper_rpc import Flipper, write_png

# view.c: arrows of 12 px, pitch 14, top row at y = 34.
SIZE, PITCH, ARROW_Y = 12, 14, 34
DIRECTIONS = ("up", "down", "left", "right")


def arrow_mask(direction: str, filled: bool) -> set[tuple[int, int]]:
    """Pixels of view.c:draw_arrow, as (x, y) offsets in the 12x12 box."""
    spans = []
    mid, last = SIZE // 2, SIZE - 1
    for v in range(mid):
        u1, u2 = mid - 1 - v, mid + v
        if filled:
            spans.append((u1, u2, v))
        elif v == mid - 1:
            spans += [(u1, 4, v), (7, u2, v)]
        else:
            spans += [(u1, u1, v), (u2, u2, v)]
    for v in range(mid, SIZE):
        if filled or v == last:
            spans.append((4, 7, v))
        else:
            spans += [(4, 4, v), (7, 7, v)]
    pixels = set()
    for u1, u2, v in spans:
        for u in range(u1, u2 + 1):
            pixels.add(
                {
                    "up": (u, v),
                    "down": (u, last - v),
                    "left": (v, u),
                    "right": (last - v, u),
                }[direction]
            )
    return pixels


TEMPLATES = [(d, f, arrow_mask(d, f)) for d in DIRECTIONS for f in (True, False)]


def read_sequence(pixels) -> list[tuple[str, bool]] | None:
    """The arrows on screen as (direction, entered), or None if there are none."""
    for length in range(8, 2, -1):
        x0 = 64 - (length * PITCH - (PITCH - SIZE)) // 2
        arrows = []
        for i in range(length):
            x = x0 + i * PITCH
            box = {
                (dx, dy)
                for dy in range(SIZE)
                for dx in range(SIZE)
                if pixels[ARROW_Y + dy][x + dx]
            }
            match = next(((d, f) for d, f, mask in TEMPLATES if mask == box), None)
            if match is None:
                break
            arrows.append(match)
        else:
            return arrows
    return None


def wait_for(flipper: Flipper, predicate, timeout: float = 5.0):
    end = time.time() + timeout
    while time.time() < end:
        flipper.poll()
        value = predicate(flipper.pixels())
        if value:
            return value
    raise TimeoutError(predicate.__name__)


def play_until(flipper: Flipper, stop_after: int | None = None) -> None:
    """Types the arrows on screen. Stops after `stop_after` arrows, or when no
    sequence is left (end of the round)."""
    typed = 0
    while stop_after is None or typed < stop_after:
        sequence = None
        end = time.time() + 0.3
        while sequence is None and time.time() < end:
            flipper.poll()
            sequence = read_sequence(flipper.pixels())
        if sequence is None:
            return
        todo = [d for d, entered in sequence if not entered]
        if not todo:
            flipper.wait(0.05)
            continue
        before = sequence
        flipper.key(todo[0])
        typed += 1
        # Wait for the screen to show the arrow before reading again.
        end = time.time() + 1.0
        while time.time() < end:
            flipper.poll()
            if read_sequence(flipper.pixels()) != before:
                break


def main() -> None:
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "docs/screenshots")
    out.mkdir(parents=True, exist_ok=True)
    flipper = Flipper()
    flipper.start_stream()
    flipper.wait(0.5)

    def shot(name: str) -> None:
        flipper.wait(0.15)
        write_png(out / f"{name}.png", flipper.pixels())
        print(f"{name}.png")

    shot("titre")
    flipper.key("ok")
    flipper.wait(0.4)
    shot("annonce")
    # Two stratagems and a half: crossed icons and arrows half entered.
    for _ in range(2):
        sequence = wait_for(flipper, read_sequence)
        play_until(flipper, len(sequence))
    play_until(flipper, 2)
    shot("manche")
    play_until(flipper)
    flipper.wait(1.6)  # bonuses show one by one, every 500 ms
    shot("fin-de-manche")
    flipper.key("back")
    flipper.wait(0.3)
    shot("pause")
    flipper.key("ok")  # back to the round end
    flipper.wait(0.2)
    flipper.key("ok")  # next round: let the timer run out
    flipper.wait(1.5 + 10.5)
    shot("game-over")
    flipper.close()


if __name__ == "__main__":
    main()
