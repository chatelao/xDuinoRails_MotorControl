# Seeed Studio XIAO ESP32-C3

The Seeed Studio XIAO ESP32-C3 is a powerful, low-power microcontroller in a tiny form factor. It is based on the Espressif ESP32-C3 chip, which has a 32-bit RISC-V chip processor with a four-stage pipeline that operates at up to 160 MHz.

## Hardware Overview

### Features

*   **Powerful MCU:** 32-bit RISC-V single-core processor that operates at up to 160 MHz
*   **Complete WiFi subsystem:** Complies with IEEE 802.11b/g/n protocol and supports Station mode, SoftAP mode, SoftAP + Station mode, and promiscuous mode
*   **Bluetooth LE subsystem:** Supports features of Bluetooth 5 and Bluetooth mesh
*   **Ultra-Low Power:** Deep sleep power consumption is about 43μA
*   **Better RF performance:** External RF antenna included
*   **Battery charging chip:** Supports lithium battery charge and discharge management
*   **Rich on-chip resources:** 400KB of SRAM, and 4MB of on-board flash memory
*   **Ultra small size:** As small as a thumb(21x17.8mm) XIAO series classic form-factor for wearable devices and small projects
*   **Reliable security features:** Cryptographic hardware accelerators that support AES-128/256, Hash, RSA, HMAC, digital signature and secure boot
*   **Rich interfaces:** 1xI2C, 1xSPI, 2xUART, 11xGPIO(PWM), 4xADC, 1xJTAG bonding pad interface
*   **Single-sided components, surface mounting design**

### Specifications

| Item                                     | Value                                           |
| ---------------------------------------- | ----------------------------------------------- |
| CPU                                      | ESP32-C3 32-bit RISC-V @160MHz |
| Flash Memory                             | 4MB                                             |
| SRAM                                     | 400KB                                           |
| Digital I/O Pins                         | 11                                              |
| Analog I/O Pins                          | 4                                               |
| PWM Pins                                 | 11                                              |
| I2C interface                            | 1                                               |
| SPI interface                            | 1                                               |
| UART interface                           | 2                                               |
| Power supply and downloading interface | Type-C                                          |
| Power                                    | 3.3V/5V DC                                      |
| Dimensions                               | 21×17.8×3.5mm                                   |

## Pinout

![XIAO ESP32-C3 Pinout](https://files.seeedstudio.com/wiki/XIAO_WiFi/pin_map-2.png)

### External Pins

| Pin | Arduino Pin | GPIO  | Function        | Description                               |
| --- | ----------- | ----- | --------------- | ----------------------------------------- |
| 1   | D0          | GPIO2 | ADC, PWM        | Digital I/O, Analog Input 0               |
| 2   | D1          | GPIO3 | ADC, PWM        | Digital I/O, Analog Input 1               |
| 3   | D2          | GPIO4 | ADC, PWM        | Digital I/O, Analog Input 2               |
| 4   | D3          | GPIO5 | ADC, PWM        | Digital I/O, Analog Input 3               |
| 5   | D4 / SDA    | GPIO6 | I2C_SDA, PWM    | I2C Data Pin                              |
| 6   | D5 / SCL    | GPIO7 | I2C_SCL, PWM    | I2C Clock Pin                             |
| 7   | D6 / TX     | GPIO21| UART_TX, PWM    | UART Transmit Pin                         |
| 8   | D7 / RX     | GPIO20| UART_RX, PWM    | UART Receive Pin                          |
| 9   | D8 / SCK    | GPIO8 | SPI_SCK, PWM    | SPI Clock Pin                             |
| 10  | D9 / MISO   | GPIO9 | SPI_MISO, PWM   | SPI Master In Slave Out Pin               |
| 11  | D10 / MOSI  | GPIO10| SPI_MOSI, PWM   | SPI Master Out Slave In Pin               |
| 12  | 3V3         | -     | Power           | 3.3V Power Output                         |
| 13  | GND         | -     | Ground          | Ground                                    |
| 14  | 5V          | -     | Power           | 5V Power Input/Output                     |

### On-board Devices

| Device           | GPIO Pin | Description                                                                |
| ---------------- | -------- | -------------------------------------------------------------------------- |
| Charge LED       | -        | A single LED that indicates the battery charging status.                   |
| Boot Button      | -        | Used to enter bootloader mode for flashing firmware. Not a user-programmable button. |
| Reset Button     | -        | Resets the microcontroller. Not a user-programmable button.                |

## Getting Started

### Arduino IDE

1.  **Install the Arduino IDE:** Download and install the latest version from the [Arduino website](https://www.arduino.cc/en/software).
2.  **Add the Board Manager URL:** In the Arduino IDE, go to `File > Preferences` and add the following URL to the "Additional Boards Manager URLs" field:
    ```
    https://jihulab.com/esp-mirror/espressif/arduino-esp32.git
    ```
3.  **Install the Board Package:** Go to `Tools > Board > Boards Manager`, search for "esp32", and install the "esp32" package.
4.  **Select the Board and Port:** Go to `Tools > Board > ESP32 Arduino` and select "XIAO_ESP32C3". Then, select the correct port from the `Tools > Port` menu.
5.  **Upload a Sketch:** Open the "Blink" example from `File > Examples > 01.Basics > Blink`. You will need to modify the code to use a pin connected to an external LED, as there is no built-in user LED.

### PlatformIO

1.  **Install PlatformIO:** Follow the instructions on the [PlatformIO website](https://platformio.org/install) to install the PlatformIO IDE extension for your editor of choice (e.g., VSCode).
2.  **Create a New Project:** Create a new PlatformIO project, selecting "Seeed Studio XIAO ESP32C3" as the board.
3.  **Configure `platformio.ini`:** Your `platformio.ini` file should look something like this:
    ```ini
    [env:seeed_xiao_esp32c3]
    platform = espressif32
    board = seeed_xiao_esp32c3
    framework = arduino
    ```
4.  **Write and Upload Code:** Write your code in the `src/main.cpp` file. Use the PlatformIO "Upload" task to flash the board.

### MicroPython/CircuitPython

1.  **Download the Firmware:** Download the latest MicroPython or CircuitPython firmware for the XIAO ESP32C3 from their respective websites.
2.  **Enter Bootloader Mode:** Press and hold the "BOOT" button while connecting the XIAO ESP32C3 to your computer.
3.  **Flash the Firmware:** Use the appropriate flashing tool to upload the firmware to the board.
4.  **Connect to the REPL:** Use a serial terminal program (e.g., Thonny, PuTTY) to connect to the board's serial port. You should see the MicroPython/CircuitPython REPL prompt.

## Resources

*   [Seeed Studio XIAO ESP32-C3 Wiki](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)
*   [ESP32-C3 Datasheet](https://files.seeedstudio.com/wiki/XIAO_WiFi/Resources/esp32-c3_datasheet.pdf)
*   [Seeed Studio XIAO ESP32-C3 Schematic](https://files.seeedstudio.com/wiki/XIAO_WiFi/Resources/Seeeduino-XIAO-ESP32C3-SCH.pdf)
*   [XIAO ESP32-C3 Eagle and KiCAD files](https://files.seeedstudio.com/wiki/XIAO_WiFi/Resources/XIAO-ESP32C3-v1.2_SCH-PCB.zip)
