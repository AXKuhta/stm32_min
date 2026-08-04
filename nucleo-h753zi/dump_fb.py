#!/usr/bin/env python3
"""Dump the 320x240 RGB565 framebuffer from the target with pyOCD."""

import argparse
import time
from pathlib import Path

from pyocd.core.helpers import ConnectHelper
from pyocd.core.target import Target

FB_ADDRESS = 0x24000618
FB_SIZE = 320 * 240 * 2
DCMI_CR = 0x48020000
DCMI_CR_CAPTURE = 1 << 0


def auto_int(value: str) -> int:
    return int(value, 0)


def display_rgb565(data: bytes, width: int, height: int, byte_order: str) -> None:
    try:
        import matplotlib.pyplot as plt
        import numpy as np
    except ImportError as error:
        raise SystemExit("Display requires: python -m pip install numpy matplotlib") from error

    required_size = width * height * 2
    if len(data) < required_size:
        raise ValueError(f"display needs {required_size} bytes, but only {len(data)} were read")

    dtype = ">u2" if byte_order == "big" else "<u2"
    pixels = np.frombuffer(data, dtype=dtype, count=width * height).reshape(height, width)
    red = (pixels >> 11) & 0x1F
    green = (pixels >> 5) & 0x3F
    blue = pixels & 0x1F

    rgb = np.empty((height, width, 3), dtype=np.uint8)
    rgb[..., 0] = (red << 3) | (red >> 2)
    rgb[..., 1] = (green << 2) | (green >> 4)
    rgb[..., 2] = (blue << 3) | (blue >> 2)

    plt.imshow(rgb)
    plt.title(f"RGB565 framebuffer ({width}x{height}, {byte_order}-endian)")
    plt.axis("off")
    plt.tight_layout()
    plt.show()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("output", nargs="?", default="frame.rgb565")
    parser.add_argument("--address", type=auto_int, default=FB_ADDRESS)
    parser.add_argument("--size", type=auto_int, default=FB_SIZE)
    parser.add_argument("--probe", help="probe unique ID when multiple probes are connected")
    parser.add_argument("--target", default="cortex_m", help="pyOCD target name (default: cortex_m)")
    parser.add_argument("--width", type=int, default=320)
    parser.add_argument("--height", type=int, default=240)
    parser.add_argument("--byte-order", choices=("big", "little"), default="big")
    parser.add_argument("--no-display", action="store_true")
    parser.add_argument("--leave-halted", action="store_true")
    args = parser.parse_args()

    connect_args = {
        "target_override": args.target,
        "options": {"connect_mode": "attach"},
    }
    if args.probe:
        connect_args["unique_id"] = args.probe

    with ConnectHelper.session_with_chosen_probe(**connect_args) as session:
        target = session.target
        was_running = target.get_state() == Target.State.RUNNING
        target.halt()

        try:
            # The CPU is halted, but an in-progress DCMI snapshot/DMA can finish.
            deadline = time.monotonic() + 2.0
            while target.read32(DCMI_CR) & DCMI_CR_CAPTURE:
                if time.monotonic() >= deadline:
                    raise TimeoutError("DCMI capture did not finish within 2 seconds")
                time.sleep(0.005)

            data = bytearray()
            for offset in range(0, args.size, 4096):
                count = min(4096, args.size - offset)
                data.extend(target.read_memory_block8(args.address + offset, count))

            Path(args.output).write_bytes(data)
            print(f"Wrote {len(data)} bytes from {args.address:#010x} to {args.output}")
        finally:
            if was_running and not args.leave_halted:
                target.resume()

    if not args.no_display:
        display_rgb565(data, args.width, args.height, args.byte_order)


if __name__ == "__main__":
    main()
