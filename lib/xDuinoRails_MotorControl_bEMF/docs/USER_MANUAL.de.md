# Benutzerhandbuch

Willkommen zum Benutzerhandbuch für die Bibliothek `xDuinoRails_MotorControl_bEMF`. Dieses Dokument bietet einen umfassenden Überblick über die Funktionen der Bibliothek und deren effektive Nutzung.

## Einführung

Die Bibliothek `xDuinoRails_MotorControl_bEMF` ist eine Hardware-Abstraktionsschicht (HAL), die für die Low-Level-Hochleistungs-Motorsteuerung im Arduino-Ökosystem entwickelt wurde. Sie bietet eine plattformunabhängige API zur Steuerung von gebürsteten Gleichstrommotoren mit PWM und zur Messung der Gegen-EMK (BEMF) für eine sensorlose Rückkopplung.

Die Hauptmerkmale der Bibliothek sind:
- Hardwarebeschleunigte PWM-Motorsteuerung.
- DMA-basiertes ADC-Sampling für eine effiziente BEMF-Messung.
- Eine saubere, konsistente API für verschiedene Mikrocontroller.
- Echtzeit-BEMF-Datenbereitstellung über eine benutzerdefinierte Callback-Funktion.

## Kernkonzepte

Vor der Verwendung der Bibliothek ist es wichtig, einige Kernkonzepte zu verstehen:

### Hardware-Abstraktionsschicht (HAL)

Diese Bibliothek ist eine HAL, was bedeutet, dass sie eine standardisierte Schnittstelle zur zugrunde liegenden Hardware bietet. Die eigentliche Implementierung der Funktionen ist spezifisch für den von Ihnen verwendeten Mikrocontroller (z. B. ESP32, RP2040, STM32). Dies ermöglicht es Ihnen, portablen Anwendungscode zu schreiben, der nicht an eine bestimmte Hardwareplattform gebunden ist.

### Pulsweitenmodulation (PWM)

PWM ist eine Technik zur Steuerung der an den Motor gelieferten Leistung, die wiederum seine Geschwindigkeit steuert. Die Funktion `hal_motor_set_pwm()` der Bibliothek ermöglicht es Ihnen, das PWM-Tastverhältnis und die Richtung des Motors einzustellen.

### Gegen-EMK (BEMF)

BEMF ist eine Spannung, die von einem sich drehenden Motor erzeugt wird. Die Größe dieser Spannung ist proportional zur Motordrehzahl. Durch Messen der BEMF ist es möglich, die Motordrehzahl ohne einen separaten Sensor (z. B. einen Drehgeber) zu bestimmen. Dies wird als sensorlose Motorsteuerung bezeichnet.

### Direkter Speicherzugriff (DMA)

Die Bibliothek verwendet DMA, um ADC-Abtastwerte der BEMF in einen Speicherpuffer zu übertragen, ohne die CPU zu belasten. Dies ist hocheffizient und ermöglicht eine Hochfrequenzabtastung, ohne die Leistung der Hauptanwendung zu beeinträchtigen.

## API-Referenz

Eine detaillierte Beschreibung aller Bibliotheksfunktionen finden Sie in der [Entwicklerreferenz](DEVELOPER_REFERENCE.de.md).

## Erste Schritte

Um mit der Bibliothek zu beginnen, empfehlen wir Ihnen, die Schritte in der Anleitung [Wie man es benutzt](HOW_TO_USE.de.md) zu befolgen. Praktische Beispiele finden Sie auch im `examples`-Verzeichnis dieses Projekts.

## Fehlerbehebung

- **Motor bewegt sich nicht:** Überprüfen Sie Ihre Verkabelung und die Pin-Definitionen, die Sie an `hal_motor_init()` übergeben. Stellen Sie sicher, dass das Tastverhältnis auf einen Wert ungleich Null eingestellt ist.
- **Keine BEMF-Daten:** Überprüfen Sie, ob die BEMF-Pins korrekt an die Motorklemmen angeschlossen sind und ob Ihre Callback-Funktion korrekt implementiert und registriert ist.
- **Kompilierungsfehler:** Stellen Sie sicher, dass Sie die richtige PlatformIO-Umgebung für Ihr Ziel-Board ausgewählt haben, da dies bestimmt, welche HAL-Implementierung verwendet wird.
