# Le Roomba (french_robot)

*My first robot: a French robot vacuum I tore open to see how it works, then rebuilt myself.*

<!-- VIDEO: on github.com, edit this README, delete this comment, and drag le_roomba_driving.mp4 here. GitHub will paste a https://github.com/user-attachments/assets/... link that plays inline. -->

*Le Roomba driving on Wi-Fi teleop.*

## Why I built it

I wanted to know how a robot vacuum actually works. How does it drive with only two wheels? How does it know it's about to hit a wall? So I opened one up, kept the chassis and drive motors, and replaced the brains with my own.

It looks like a kid's creation, but it's mine, it works, and I had fun with it. It also taught me what was missing: avoiding walls isn't the same as knowing where you are. That gap is what my next robot, [Bluebot](https://github.com/NinjaGirlGmr/bluebot), was built to close.

## What it does

- **Wi-Fi teleop:** I drive it with an Xbox controller from my laptop. The controller app sends drive commands to the robot over Wi-Fi (UDP).
- **Collision blocking:** an ultrasonic sensor on the front watches for obstacles. If something is within 20 cm, the robot refuses to drive forward but still lets me turn or back out.
- **Safety stop:** if no command arrives for 5 seconds (for example, the controller disconnects), the motors coast to a stop.
- **Status screen:** a small OLED shows the Wi-Fi network, IP address, ultrasonic distance, and whether forward motion is blocked.


https://github.com/user-attachments/assets/344f34f3-b923-4eee-900c-8ba5960d1558




## Hardware

| Part | Role |
|---|---|
| Salvaged robot vacuum chassis | Body, two drive wheels and motors |
| Raspberry Pi Pico 2 W | Main controller, Wi-Fi |
| L298N motor driver | Drives both wheel motors (20 kHz PWM) |
| HC-SR04 ultrasonic sensor | Front obstacle distance |
| SSD1306 OLED (128×64, I2C) | Status display |
| LDS-style lidar (UART) | Parser written; not yet used for driving |
| Xbox controller + laptop | Teleop input |

Pin assignments are all in `constants.h`.

## How it works

1. **Controller app** (`xbox_controller/`, Python + pygame): reads the left stick for forward/back and the right stick for turning, mixes them into left and right wheel speeds, and sends `DRIVE <left> <right>` packets to the robot on UDP port 42424. It applies a deadzone, scales down turning so it isn't twitchy, resends every 0.1 s so the robot doesn't time out, and sends `STOP` if the controller disconnects or the app closes.
2. **Robot firmware** (`french_robot.c`, C with the Pico SDK): connects to Wi-Fi, listens for those packets, clamps the speeds, and drives the motors through the L298N.
3. **Round-robin main loop** (`main`, `french_robot.c`): instead of one task waiting on another, the loop takes turns servicing each job on its own timer: poll the ultrasonic sensor (every 200 ms), check the safety timeout, log a heartbeat (every 1 s), and refresh the status screen (every 1 s). Then it sleeps 2 ms and goes around again, so the robot stays responsive to new drive commands.
4. **Collision check** (`blocked_by_ultrasonic`, `french_robot.c`): before every forward command, it checks the latest ultrasonic reading. At 20 cm or closer it blocks forward motion; it only releases once the distance is back above 22 cm, so it doesn't flicker on and off at the edge. Turning and reversing are always allowed, so I can drive away from whatever is in front. Stale readings (older than 0.4 s) are ignored.

## Code layout

| File | What it does |
|---|---|
| `french_robot.c` | Main loop: Wi-Fi, UDP command handling, collision check, safety timeout, status screen |
| `motor.c` | L298N motor control: speed, direction, coast, brake |
| `ultrasonic.c` | HC-SR04 driver: triggers a pulse every 200 ms and measures the echo |
| `ssd1306.c` | OLED driver and text drawing |
| `lidar.c` | Parser for 42-byte LDS lidar frames into a 360° distance buffer |
| `joystick.c` | Earlier analog-joystick input (not in the current build) |
| `xbox_controller/` | Laptop-side Xbox controller client |

## Current status

- Working: Wi-Fi teleop, ultrasonic collision blocking, safety timeout, status screen.
- Not finished: the lidar parser is written but isn't wired into the main loop yet, so lidar-based blocking is turned off. The controller can send an `AVOID TOGGLE` command, but the robot doesn't act on it yet.

## Building

1. Install the Raspberry Pi Pico SDK (2.2.0) or the Pico VS Code extension.
2. Create `wifi_config.h` with your network (it's git-ignored, so credentials never get committed):
   ```c
   #define WIFI_SSID "your-network"
   #define WIFI_PASSWORD "your-password"
   #define WIFI_CONNECT_TIMEOUT_MS 10000
   ```
3. Build with CMake (board: `pico2_w`) and flash `french_robot.uf2`.
4. Run the controller: `python -m xbox_controller --host <robot_ip>` from `xbox_controller/` (see its README).

## Development notes
All displayed code, documentation, and implementation were done by me. No AI was used in the process.
