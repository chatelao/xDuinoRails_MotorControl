# Motor Serial Debug Example

This example demonstrates how to control the motor speed and direction using commands sent via the Serial Monitor.

## How it Works

The sketch initializes the motor driver and listens for characters on the Serial port (115200 baud).

- **Speed Control:** Send digits `0` through `9` to set the motor speed.
  - `0`: 0% Speed (Stop)
  - `1`: 10% Speed
  - ...
  - `9`: 90% Speed
- **Direction Control:** Send `-` (hyphen) to toggle the motor direction between Forward and Reverse.

## Hardware Setup

The pin assignments are automatically selected based on the board definition in `platformio.ini`.

| Board | PWM A | PWM B | BEMF A | BEMF B |
|-------|-------|-------|--------|--------|
| **Seeed XIAO RP2040** | D9 | D10 | D7 | D8 |
| **Nucleo G431RB** | D7 | D8 | A1 | A3 |
| **Generic/Other** | 7 | 8 | A3 | A2 |

**Note:** Ensure your motor driver and power supply are correctly connected to the specified pins.

## Usage

1. Flash the firmware to your board.
2. Open the Serial Monitor (115200 baud).
3. Type a command (e.g., `5`) and press Send (or Enter).
4. The motor should respond, and the new state will be printed to the console.
