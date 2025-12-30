# Xbox Controller

Python client for driving the robot over Wi-Fi using an Xbox controller.

## Setup
1) python -m venv .venv
2) source .venv/bin/activate
3) pip install -r requirements.txt

## Running
- `python -m xbox_controller --host <robot_ip> [--port 42424] [--rate 90] [--deadzone 0.08] [--turn-scale 0.5] [--resend-interval 0.3] [--forward-axis 1] [--turn-axis 2]`
- Example: `python -m xbox_controller --host 192.168.1.120`

## Notes
- Uses pygame for controller input; make sure your Xbox controller is connected via Bluetooth and visible in macOS.
- Mapping: left stick Y = forward/back; right stick X = turn. Motors receive normalized speeds in range [-1, 1]. Reduce right-stick sensitivity with `--turn-scale` if forward should dominate. Override axis indices with `--forward-axis` / `--turn-axis` if your controller reports differently (common alternative: turn-axis 2 for right stick X, 3 for right stick Y).
- The client sends UDP packets of the form `DRIVE <left> <right>`; it rate-limits to ~90 Hz by default and resends unchanged values every ~0.3s to avoid robot idle timeout (rounded to 3 decimals). It sends `STOP` on exit or disconnect.
