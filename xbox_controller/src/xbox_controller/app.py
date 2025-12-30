"""Xbox controller UDP client for driving the robot."""
from __future__ import annotations

import argparse
import socket
import sys
from pathlib import Path

import pygame

DEFAULT_CONFIG = Path("config.json")
DEFAULT_PORT = 42424
DEFAULT_RATE_HZ = 90.0
DEFAULT_DEADZONE = 0.08
DEFAULT_TURN_SCALE = 0.5
# Axis indices for Xbox controller (pygame/SDL typical):
#  axis 1 = left stick Y (forward/back)
#  axis 2 = right stick X (turn)  # some mappings use 3 for right stick Y
DEFAULT_FORWARD_AXIS = 1
DEFAULT_TURN_AXIS = 2
DEFAULT_RESEND_INTERVAL = 0.3  # seconds; resend unchanged values to keep robot alive


def clamp(value: float, lo: float = -1.0, hi: float = 1.0) -> float:
    return max(lo, min(hi, value))


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description="Stream Xbox controller values to the robot over UDP")
    parser.add_argument("--host", required=True, help="Robot IP or hostname")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help="UDP port on the robot")
    parser.add_argument("--rate", type=float, default=DEFAULT_RATE_HZ, help="Send frequency in Hz")
    parser.add_argument("--deadzone", type=float, default=DEFAULT_DEADZONE, help="Axis deadzone threshold")
    parser.add_argument(
        "--turn-scale",
        type=float,
        default=DEFAULT_TURN_SCALE,
        help="Scale factor for left/right sensitivity (smaller = less twitchy)",
    )
    parser.add_argument(
        "--resend-interval",
        type=float,
        default=DEFAULT_RESEND_INTERVAL,
        help="Resend unchanged values after this many seconds to avoid robot idle timeout",
    )
    parser.add_argument(
        "--forward-axis",
        type=int,
        default=DEFAULT_FORWARD_AXIS,
        help="Joystick axis index for forward/back (left stick Y on most Xbox pads)",
    )
    parser.add_argument(
        "--turn-axis",
        type=int,
        default=DEFAULT_TURN_AXIS,
        help="Joystick axis index for turning (right stick X on most Xbox pads)",
    )
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG, help="Unused placeholder for future config")
    return parser.parse_args(argv)


def init_controller() -> pygame.joystick.Joystick:
    pygame.joystick.init()
    if pygame.joystick.get_count() == 0:
        print("No controller detected. Connect an Xbox controller and try again.")
        sys.exit(1)

    js = pygame.joystick.Joystick(0)
    js.init()
    print(f"Using controller: {js.get_name()}")
    return js


def axes_to_drive(
    js: pygame.joystick.Joystick,
    deadzone: float,
    turn_scale: float,
    forward_axis: int,
    turn_axis: int,
) -> tuple[float, float]:
    # Mapping: forward/back from left stick Y; turn from right stick X.
    forward = -js.get_axis(forward_axis)  # invert so up is positive
    turn = js.get_axis(turn_axis)

    if abs(forward) < deadzone:
        forward = 0.0
    if abs(turn) < deadzone:
        turn = 0.0
    turn *= turn_scale

    left = clamp(forward + turn)
    right = clamp(forward - turn)
    return left, right


def send_drive(sock: socket.socket, addr: tuple[str, int], left: float, right: float) -> None:
    message = f"DRIVE {left:.3f} {right:.3f}".encode()
    sock.sendto(message, addr)


def send_stop(sock: socket.socket, addr: tuple[str, int]) -> None:
    sock.sendto(b"STOP", addr)


def main(argv=None):
    args = parse_args(argv)

    pygame.init()
    clock = pygame.time.Clock()
    controller = init_controller()

    dest = (args.host, args.port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    print(f"Sending DRIVE packets to {dest} at {args.rate} Hz. Ctrl+C to stop.")
    last_sent = (None, None)
    last_sent_time = 0.0

    try:
        while True:
            for event in pygame.event.get():
                if event.type == pygame.JOYDEVICEADDED:
                    print("Controller reconnected.")
                if event.type == pygame.JOYDEVICEREMOVED:
                    print("Controller disconnected; stopping motors.")
                    send_stop(sock, dest)
                    sys.exit(1)

            left, right = axes_to_drive(
                controller,
                args.deadzone,
                args.turn_scale,
                args.forward_axis,
                args.turn_axis,
            )
            current = (round(left, 3), round(right, 3))
            now = pygame.time.get_ticks() / 1000.0
            if current != last_sent or (now - last_sent_time) >= args.resend_interval:
                send_drive(sock, dest, left, right)
                last_sent = current
                last_sent_time = now
            clock.tick(args.rate)
    except KeyboardInterrupt:
        print("Stopping motors and exiting...")
        send_stop(sock, dest)
    finally:
        pygame.quit()


if __name__ == "__main__":
    main()
