# Seeed Studio XIAO RP2040

The Seeed Studio XIAO RP2040 is a powerful, low-power microcontroller in a tiny form factor. It is based on the Raspberry Pi RP2040 chip, which has a dual-core ARM Cortex M0+ processor running at up to 133 MHz. The board also has 264KB of SRAM and 2MB of on-board Flash memory.

## Hardware Overview

### Features

*   **Powerful MCU:** Dual-core ARM Cortex M0+ processor, flexible clock running up to 133 MHz
*   **Rich on-chip resources:** 264KB of SRAM, and 2MB of on-board Flash memory
*   **Flexible compatibility:** Support Micropython/Arduino/CircuitPython
*   **Easy project operation:** Breadboard-friendly & SMD design, no components on the back
*   **Small size:** As small as a thumb(21x17.8mm) for wearable devices and small projects.
*   **Multiple interfaces:** 11 digital pins, 4 analog pins, 11 PWM Pins,1 I2C interface, 1 UART interface, 1 SPI interface, 1 SWD Bonding pad interface.

### Specifications

| Item                                     | Value                                           |
| ---------------------------------------- | ----------------------------------------------- |
| CPU                                      | Dual-core ARM Cortex M0+ processor up to 133MHz |
| Flash Memory                             | 2MB                                             |
| SRAM                                     | 264KB                                           |
| Digital I/O Pins                         | 11                                              |
| Analog I/O Pins                          | 4                                               |
| PWM Pins                                 | 11                                              |
| I2C interface                            | 1                                               |
| SPI interface                            | 1                                               |
| UART interface                           | 1                                               |
| Power supply and downloading interface | Type-C                                          |
| Power                                    | 3.3V/5V DC                                      |
| Dimensions                               | 21×17.8×3.5mm                                   |

## Pinout

![XIAO RP2040 Pinout](https://files.seeedstudio.com/wiki/XIAO-RP2040/img/xinpin.jpg)

### External Pins

| Pin | Arduino Pin | GPIO  | Function        | Description                               |
| --- | ----------- | ----- | --------------- | ----------------------------------------- |
| 1   | D0          | GPIO26| ADC0, PWM       | Digital I/O, Analog Input 0               |
| 2   | D1          | GPIO27| ADC1, PWM       | Digital I/O, Analog Input 1               |
| 3   | D2          | GPIO28| ADC2, PWM       | Digital I/O, Analog Input 2               |
| 4   | D3          | GPIO29| ADC3, PWM       | Digital I/O, Analog Input 3               |
| 5   | D4 / SDA    | GPIO6 | I2C0_SDA, PWM   | I2C0 Data Pin                             |
| 6   | D5 / SCL    | GPIO7 | I2C0_SCL, PWM   | I2C0 Clock Pin                            |
| 7   | D6 / TX     | GPIO0 | UART0_TX, PWM   | UART0 Transmit Pin                        |
| 8   | D7 / RX     | GPIO1 | UART0_RX, PWM   | UART0 Receive Pin                         |
| 9   | D8 / SCK    | GPIO2 | SPI0_SCK, PWM   | SPI0 Clock Pin                            |
| 10  | D9 / MISO   | GPIO4 | SPI0_RX, PWM    | SPI0 Master In Slave Out Pin              |
| 11  | D10 / MOSI  | GPIO3 | SPI0_TX, PWM    | SPI0 Master Out Slave In Pin              |
| 12  | 3V3         | -     | Power           | 3.3V Power Output                         |
| 13  | GND         | -     | Ground          | Ground                                    |
| 14  | 5V          | -     | Power           | 5V Power Input/Output                     |

### On-board Devices

| Device           | GPIO Pin | Description                                                                |
| ---------------- | -------- | -------------------------------------------------------------------------- |
| User LED (Red)   | GPIO17   | A single red LED that can be controlled by the user (active low).          |
| User LED (Green) | GPIO16   | A single green LED that can be controlled by the user (active low).        |
| User LED (Blue)  | GPIO25   | A single blue LED that can be controlled by the user (active low).         |
| RGB LED (Neopixel) | GPIO12   | WS2812 addressable RGB LED. Data pin.                                      |
| RGB LED Power    | GPIO11   | Power enable for the RGB LED. Must be set HIGH to enable the Neopixel.     |
| Boot Button      | -        | Used to enter bootloader mode for flashing firmware. Not a user-programmable button. |
| Reset Button     | -        | Resets the microcontroller. Not a user-programmable button.                |

## Getting Started

### Arduino IDE

1.  **Install the Arduino IDE:** Download and install the latest version from the [Arduino website](https://www.arduino.cc/en/software).
2.  **Add the Board Manager URL:** In the Arduino IDE, go to `File > Preferences` and add the following URL to the "Additional Boards Manager URLs" field:
    ```
    https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
    ```
3.  **Install the Board Package:** Go to `Tools > Board > Boards Manager`, search for "RP2040", and install the "Raspberry Pi Pico/RP2040" package.
4.  **Select the Board and Port:** Go to `Tools > Board` and select "Seeed Studio XIAO RP2040". Then, select the correct port from the `Tools > Port` menu.
5.  **Upload a Sketch:** Open the "Blink" example from `File > Examples > 01.Basics > Blink`. You can control the on-board LEDs by using the GPIO numbers (16, 17, 25) as the pin number in your code.

### PlatformIO

1.  **Install PlatformIO:** Follow the instructions on the [PlatformIO website](https://platformio.org/install) to install the PlatformIO IDE extension for your editor of choice (e.g., VSCode).
2.  **Create a New Project:** Create a new PlatformIO project, selecting "Seeed Studio XIAO RP2040" as the board.
3.  **Configure `platformio.ini`:** Your `platformio.ini` file should look something like this:
    ```ini
    [env:seeed_xiao_rp2040]
    platform = raspberrypi
    board = seeed_xiao_rp2040
    framework = arduino
    ```
4.  **Write and Upload Code:** Write your code in the `src/main.cpp` file. Use the PlatformIO "Upload" task to flash the board.

### MicroPython/CircuitPython

1.  **Download the Firmware:** Download the latest MicroPython or CircuitPython firmware for the XIAO RP2040 from their respective websites.
2.  **Enter Bootloader Mode:** Press and hold the "BOOT" button while connecting the XIAO RP2040 to your computer. It should appear as a mass storage device named "RPI-RP2".
3.  **Flash the Firmware:** Drag and drop the downloaded `.uf2` firmware file onto the "RPI-RP2" drive. The board will automatically reboot and run the new firmware.
4.  **Connect to the REPL:** Use a serial terminal program (e.g., Thonny, PuTTY) to connect to the board's serial port. You should see the MicroPython/CircuitPython REPL prompt.

## Resources

*   [Seeed Studio XIAO RP2040 Wiki](https://wiki.seeedstudio.com/XIAO-RP2040/)
*   [RP2040 Datasheet](https://files.seeedstudio.com/wiki/XIAO-RP2040/res/rp2040_datasheet.pdf)
*   [Seeed Studio XIAO RP2040 Schematic](https://files.seeedstudio.com/wiki/XIAO-RP2040/res/Seeed-Studio-XIAO-RP2040-v1.3.pdf)
*   [XIAO RP2040 Eagle and KiCAD files](https://wiki.seeedstudio.com/XIAO-RP2040/#resources)
