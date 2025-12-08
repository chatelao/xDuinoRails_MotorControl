# Seeed Studio XIAO ESP32-C3

Der Seeed Studio XIAO ESP32-C3 ist ein leistungsstarker, energiesparender Mikrocontroller im Miniaturformat. Er basiert auf dem Espressif ESP32-C3 Chip, einem 32-Bit RISC-V Prozessor, der mit bis zu 160 MHz arbeitet.

## Hardware-Übersicht

### Merkmale

*   **Leistungsstarke MCU:** 32-Bit RISC-V Single-Core-Prozessor mit bis zu 160 MHz
*   **Komplettes WiFi-Subsystem:** IEEE 802.11b/g/n konform
*   **Bluetooth LE Subsystem:** Bluetooth 5 und Bluetooth Mesh Unterstützung
*   **Ultra-Low Power:** Deep-Sleep Stromverbrauch ca. 43μA
*   **Bessere RF-Leistung:** Externe RF-Antenne inklusive
*   **Batterielade-Chip:** Unterstützt Lithium-Batterie Lade- und Entlademanagement
*   **Reichhaltige On-Chip-Ressourcen:** 400KB SRAM und 4MB On-Board Flash
*   **Ultra kleine Größe:** Daumengroß (21x17.8mm), klassischer XIAO Formfaktor
*   **Sicherheitsfunktionen:** Krypto-Hardware-Beschleuniger für AES-128/256, Hash, RSA, HMAC, digitale Signatur und Secure Boot
*   **Vielfältige Schnittstellen:** 1xI2C, 1xSPI, 2xUART, 11xGPIO(PWM), 4xADC, 1xJTAG Bonding Pad

### Spezifikationen

| Element                                  | Wert                                            |
| ---------------------------------------- | ----------------------------------------------- |
| CPU                                      | ESP32-C3 32-Bit RISC-V @160MHz                  |
| Flash-Speicher                           | 4MB                                             |
| SRAM                                     | 400KB                                           |
| Digitale I/O-Pins                        | 11                                              |
| Analoge I/O-Pins                         | 4                                               |
| PWM-Pins                                 | 11                                              |
| I2C-Schnittstelle                        | 1                                               |
| SPI-Schnittstelle                        | 1                                               |
| UART-Schnittstelle                       | 2                                               |
| Stromversorgung und Download             | Typ-C                                           |
| Stromversorgung                          | 3.3V/5V DC                                      |
| Abmessungen                              | 21×17.8×3.5mm                                   |

## Pinbelegung

![XIAO ESP32-C3 Pinbelegung](https://files.seeedstudio.com/wiki/XIAO_WiFi/pin_map-2.png)

### Externe Pins

| Pin | Arduino Pin | GPIO  | Funktion        | Beschreibung                              |
| --- | ----------- | ----- | --------------- | ----------------------------------------- |
| 1   | D0          | GPIO2 | ADC, PWM        | Digital I/O, Analog Eingang 0             |
| 2   | D1          | GPIO3 | ADC, PWM        | Digital I/O, Analog Eingang 1             |
| 3   | D2          | GPIO4 | ADC, PWM        | Digital I/O, Analog Eingang 2             |
| 4   | D3          | GPIO5 | ADC, PWM        | Digital I/O, Analog Eingang 3             |
| 5   | D4 / SDA    | GPIO6 | I2C_SDA, PWM    | I2C Daten-Pin                             |
| 6   | D5 / SCL    | GPIO7 | I2C_SCL, PWM    | I2C Takt-Pin                              |
| 7   | D6 / TX     | GPIO21| UART_TX, PWM    | UART Sende-Pin                            |
| 8   | D7 / RX     | GPIO20| UART_RX, PWM    | UART Empfangs-Pin                         |
| 9   | D8 / SCK    | GPIO8 | SPI_SCK, PWM    | SPI Takt-Pin                              |
| 10  | D9 / MISO   | GPIO9 | SPI_MISO, PWM   | SPI Master In Slave Out Pin               |
| 11  | D10 / MOSI  | GPIO10| SPI_MOSI, PWM   | SPI Master Out Slave In Pin               |
| 12  | 3V3         | -     | Strom           | 3.3V Ausgang                              |
| 13  | GND         | -     | Masse           | Masse                                     |
| 14  | 5V          | -     | Strom           | 5V Eingang/Ausgang                        |

## Erste Schritte

### Arduino IDE

1.  **Arduino IDE installieren:** Neueste Version von der [Arduino-Website](https://www.arduino.cc/en/software) laden.
2.  **Board-Manager-URL hinzufügen:** In `Datei > Voreinstellungen` folgende URL hinzufügen:
    ```
    https://jihulab.com/esp-mirror/espressif/arduino-esp32.git
    ```
3.  **Board-Paket installieren:** In `Werkzeuge > Board > Boardverwalter` nach "esp32" suchen und installieren.
4.  **Board auswählen:** `Werkzeuge > Board > ESP32 Arduino` und "XIAO_ESP32C3" wählen.

### PlatformIO

1.  **PlatformIO installieren:** VSCode-Erweiterung installieren.
2.  **Projekt erstellen:** Board "Seeed Studio XIAO ESP32C3" wählen.
3.  **`platformio.ini` konfigurieren:**
    ```ini
    [env:seeed_xiao_esp32c3]
    platform = espressif32
    board = seeed_xiao_esp32c3
    framework = arduino
    ```
4.  **Code hochladen:** Code in `src/main.cpp` schreiben und hochladen.

## Ressourcen

*   [Seeed Studio XIAO ESP32-C3 Wiki](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)
*   [ESP32-C3 Datenblatt](https://files.seeedstudio.com/wiki/XIAO_WiFi/Resources/esp32-c3_datasheet.pdf)
