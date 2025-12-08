# Seeed Studio XIAO RP2040

Der Seeed Studio XIAO RP2040 ist ein leistungsstarker, energiesparender Mikrocontroller in winzigem Format. Er basiert auf dem Raspberry Pi RP2040-Chip, der über einen Dual-Core ARM Cortex M0+ Prozessor mit bis zu 133 MHz verfügt. Das Board bietet zudem 264KB SRAM und 2MB On-Board Flash-Speicher.

## Hardware-Übersicht

### Merkmale

*   **Leistungsstarke MCU:** Dual-Core ARM Cortex M0+ Prozessor, flexibler Takt bis zu 133 MHz
*   **Reichhaltige On-Chip-Ressourcen:** 264KB SRAM und 2MB On-Board Flash-Speicher
*   **Flexible Kompatibilität:** Unterstützt Micropython/Arduino/CircuitPython
*   **Einfache Projektintegration:** Breadboard-freundliches & SMD-Design, keine Komponenten auf der Rückseite
*   **Kleine Größe:** So klein wie ein Daumen (21x17.8mm) für Wearables und kleine Projekte
*   **Vielfältige Schnittstellen:** 11 digitale Pins, 4 analoge Pins, 11 PWM-Pins, 1 I2C-Schnittstelle, 1 UART-Schnittstelle, 1 SPI-Schnittstelle, 1 SWD-Bonding-Pad-Schnittstelle.

### Spezifikationen

| Element                                  | Wert                                            |
| ---------------------------------------- | ----------------------------------------------- |
| CPU                                      | Dual-Core ARM Cortex M0+ Prozessor bis zu 133MHz|
| Flash-Speicher                           | 2MB                                             |
| SRAM                                     | 264KB                                           |
| Digitale I/O-Pins                        | 11                                              |
| Analoge I/O-Pins                         | 4                                               |
| PWM-Pins                                 | 11                                              |
| I2C-Schnittstelle                        | 1                                               |
| SPI-Schnittstelle                        | 1                                               |
| UART-Schnittstelle                       | 1                                               |
| Stromversorgung und Download-Schnittstelle | Typ-C                                         |
| Stromversorgung                          | 3.3V/5V DC                                      |
| Abmessungen                              | 21×17.8×3.5mm                                   |

## Pinbelegung

<img src="https://files.seeedstudio.com/wiki/XIAO-RP2040/img/xinpin.jpg" alt="XIAO RP2040 Pinbelegung" width="50%">

### Externe Pins

| Pin | Arduino Pin | GPIO  | Funktion        | Beschreibung                              |
| --- | ----------- | ----- | --------------- | ----------------------------------------- |
| 1   | D0          | GPIO26| ADC0, PWM       | Digital I/O, Analog Eingang 0             |
| 2   | D1          | GPIO27| ADC1, PWM       | Digital I/O, Analog Eingang 1             |
| 3   | D2          | GPIO28| ADC2, PWM       | Digital I/O, Analog Eingang 2             |
| 4   | D3          | GPIO29| ADC3, PWM       | Digital I/O, Analog Eingang 3             |
| 5   | D4 / SDA    | GPIO6 | I2C0_SDA, PWM   | I2C0 Daten-Pin                            |
| 6   | D5 / SCL    | GPIO7 | I2C0_SCL, PWM   | I2C0 Takt-Pin                             |
| 7   | D6 / TX     | GPIO0 | UART0_TX, PWM   | UART0 Sende-Pin                           |
| 8   | D7 / RX     | GPIO1 | UART0_RX, PWM   | UART0 Empfangs-Pin                        |
| 9   | D8 / SCK    | GPIO2 | SPI0_SCK, PWM   | SPI0 Takt-Pin                             |
| 10  | D9 / MISO   | GPIO4 | SPI0_RX, PWM    | SPI0 Master In Slave Out Pin              |
| 11  | D10 / MOSI  | GPIO3 | SPI0_TX, PWM    | SPI0 Master Out Slave In Pin              |
| 12  | 3V3         | -     | Strom           | 3.3V Ausgang                              |
| 13  | GND         | -     | Masse           | Masse                                     |
| 14  | 5V          | -     | Strom           | 5V Eingang/Ausgang                        |

### On-Board Geräte

| Gerät            | GPIO Pin | Beschreibung                                                               |
| ---------------- | -------- | -------------------------------------------------------------------------- |
| Benutzer-LED (Rot)| GPIO17  | Eine einzelne rote LED, steuerbar durch den Benutzer (active low).         |
| Benutzer-LED (Grün)| GPIO16 | Eine einzelne grüne LED, steuerbar durch den Benutzer (active low).        |
| Benutzer-LED (Blau)| GPIO25 | Eine einzelne blaue LED, steuerbar durch den Benutzer (active low).        |
| RGB LED (Neopixel)| GPIO12  | WS2812 adressierbare RGB-LED. Daten-Pin.                                   |
| RGB LED Strom    | GPIO11   | Stromfreigabe für die RGB-LED. Muss HIGH sein, um Neopixel zu aktivieren.  |
| Boot-Taste       | -        | Zum Starten des Bootloader-Modus. Keine programmierbare Taste.             |
| Reset-Taste      | -        | Setzt den Mikrocontroller zurück. Keine programmierbare Taste.             |

## Erste Schritte

### Arduino IDE

1.  **Arduino IDE installieren:** Laden Sie die neueste Version von der [Arduino-Website](https://www.arduino.cc/en/software) herunter.
2.  **Board-Manager-URL hinzufügen:** Gehen Sie in der Arduino IDE zu `Datei > Voreinstellungen` und fügen Sie die folgende URL zum Feld "Zusätzliche Boardverwalter-URLs" hinzu:
    ```
    https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
    ```
3.  **Board-Paket installieren:** Gehen Sie zu `Werkzeuge > Board > Boardverwalter`, suchen Sie nach "RP2040" und installieren Sie das "Raspberry Pi Pico/RP2040"-Paket.
4.  **Board und Port auswählen:** Gehen Sie zu `Werkzeuge > Board` und wählen Sie "Seeed Studio XIAO RP2040". Wählen Sie dann den korrekten Port unter `Werkzeuge > Port`.
5.  **Sketch hochladen:** Öffnen Sie das "Blink"-Beispiel unter `Datei > Beispiele > 01.Basics > Blink`. Sie können die On-Board-LEDs über die GPIO-Nummern (16, 17, 25) steuern.

### PlatformIO

1.  **PlatformIO installieren:** Folgen Sie den Anweisungen auf der [PlatformIO-Website](https://platformio.org/install), um die IDE-Erweiterung für Ihren Editor (z.B. VSCode) zu installieren.
2.  **Neues Projekt erstellen:** Erstellen Sie ein neues PlatformIO-Projekt und wählen Sie "Seeed Studio XIAO RP2040" als Board.
3.  **`platformio.ini` konfigurieren:** Ihre `platformio.ini` sollte etwa so aussehen:
    ```ini
    [env:seeed_xiao_rp2040]
    platform = raspberrypi
    board = seeed_xiao_rp2040
    framework = arduino
    ```
4.  **Code schreiben und hochladen:** Schreiben Sie Ihren Code in `src/main.cpp`. Nutzen Sie die "Upload"-Aufgabe von PlatformIO, um das Board zu flashen.

## Ressourcen

*   [Seeed Studio XIAO RP2040 Wiki](https://wiki.seeedstudio.com/XIAO-RP2040/)
*   [RP2040 Datenblatt](https://files.seeedstudio.com/wiki/XIAO-RP2040/res/rp2040_datasheet.pdf)
*   [Seeed Studio XIAO RP2040 Schaltplan](https://files.seeedstudio.com/wiki/XIAO-RP2040/res/Seeed-Studio-XIAO-RP2040-v1.3.pdf)
