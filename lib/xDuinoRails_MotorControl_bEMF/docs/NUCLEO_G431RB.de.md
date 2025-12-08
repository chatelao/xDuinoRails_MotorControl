# Nucleo G431RB Implementierung

Dieses Dokument beschreibt die Implementierungsdetails für das STM32 Nucleo G431RB Board innerhalb der `xDuinoRails_MotorControl_bEMF` Bibliothek.

## Hardware-Übersicht

Das Nucleo G431RB verfügt über den STM32G431RB Mikrocontroller, der speziell für Mixed-Signal-Anwendungen wie Motorsteuerung entwickelt wurde. Er enthält fortschrittliche analoge Peripheriegeräte wie OpAmps, Komparatoren und schnelle ADCs.

## Implementierungsdetails

### OpAmp Integration

Für eine präzise BEMF-Messung (Gegen-EMK) nutzt diese Bibliothek die internen Operationsverstärker (OpAmps) des STM32G431RB.
Die OpAmps sind im **Follower-Modus** (Einheitsverstärkungspuffer) konfiguriert. Dies bietet einen hochohmigen Eingang für die Motorklemmen, verhindert eine Beeinflussung der Spannungsmessung durch den ADC-Abtastprozess und schützt die interne ADC-Schaltung.

*   **OPAMP1:** Puffert das Signal von **PA1** (Arduino **A1**).
*   **OPAMP3:** Puffert das Signal von **PB0** (Arduino **A3**).

### ADC Konfiguration

*   **ADC1** wird verwendet, um die Ausgänge von OPAMP1 und OPAMP3 abzutasten.
*   Die Abtastung wird durch **TIM1** (Timer 1) über das TRGO (Trigger Output) Signal ausgelöst.
*   **DMA** (Direct Memory Access) wird verwendet, um die Wandlungsergebnisse in einen Ringpuffer im Speicher zu übertragen, was eine Datenerfassung ohne CPU-Belastung gewährleistet.

### PWM Erzeugung

*   **TIM1** wird zur Erzeugung der PWM-Signale für die Motorsteuerung verwendet.
*   Das Timer-Update-Ereignis synchronisiert die ADC-Abtastung und stellt sicher, dass die BEMF zum geeigneten Zeitpunkt im PWM-Zyklus (typischerweise während der AUS-Phase) gemessen wird.

## Pin-Belegung

| Funktion | Pin Name | Arduino Pin | Beschreibung |
| :--- | :--- | :--- | :--- |
| **PWM A** | PA8 | D7 | Timer 1 Kanal 1 Ausgang |
| **PWM B** | PA9 | D8 | Timer 1 Kanal 2 Ausgang |
| **BEMF A** | PA1 | A1 | OPAMP1 Nicht-invertierender Eingang |
| **BEMF B** | PB0 | A3 | OPAMP3 Nicht-invertierender Eingang |

> **Hinweis:** Stellen Sie sicher, dass Ihre Motortreiberlogik mit dieser Belegung übereinstimmt. Die OpAmp-Eingänge sollten mit den Motorklemmen (oder einem Spannungsteiler, falls die Spannung 3,3V überschreitet) verbunden werden. Da das Nucleo-Board mit 3,3V-Logik arbeitet, stellen Sie sicher, dass die BEMF-Spannung 3,3V nicht überschreitet.

### Strommessung (Optional)

Die Bibliothek unterstützt optional die Strommessung unter Verwendung von **OPAMP2** und **ADC2**.
Diese Funktion wird durch das Build-Flag `-D ENABLE_CURRENT_SENSING` aktiviert.

*   **Eingangspin:** PA7 (Arduino D11).
*   **OPAMP2:** Konfiguriert im Follower-Modus zum Puffern der Shunt-Spannung.
*   **ADC:** ADC2 tastet das gepufferte Signal synchron zur PWM ab.

## Verwendung

Um dieses Board zu verwenden, wählen Sie die Umgebung `nucleo_g431rb` in PlatformIO.

```ini
[env:nucleo_g431rb]
platform = ststm32
board = nucleo_g431rb
framework = arduino
lib_deps = ...

; Für Strommessung
[env:nucleo_g431rb_shunt]
extends = env:nucleo_g431rb
build_flags = -D ENABLE_CURRENT_SENSING
```
