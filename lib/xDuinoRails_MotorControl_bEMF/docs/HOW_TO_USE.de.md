# Wie man es benutzt

Diese Anleitung beschreibt, wie Sie die Bibliothek `xDuinoRails_MotorControl_bEMF` in Ihr Projekt integrieren und verwenden.

## 1. Einbinden der Bibliothek

Um die Bibliothek zu verwenden, müssen Sie die Haupt-Header-Datei in Ihren Quellcode einbinden:

```cpp
#include <motor_control_hal.h>
```

## 2. Initialisierung

Bevor Sie andere Bibliotheksfunktionen verwenden können, müssen Sie die Motorsteuerungs-Hardware durch Aufrufen von `hal_motor_init()` initialisieren. Diese Funktion richtet die erforderlichen Timer, PWM-Peripheriegeräte, ADC und DMA für Ihre spezifische Hardware ein.

```cpp
// Beispiel für eine BEMF-Callback-Funktion (siehe Schritt 4)
void my_bemf_callback(int raw_bemf_value) {
  // Verarbeiten der BEMF-Daten
}

void setup() {
  // Definieren Sie die GPIO-Pins für Ihre Hardware-Konfiguration
  uint8_t pwm_a_pin = 10;
  uint8_t pwm_b_pin = 11;
  uint8_t bemf_a_pin = A0;
  uint8_t bemf_b_pin = A1;

  // Initialisieren des Motorsteuerungs-HAL
  hal_motor_init(pwm_a_pin, pwm_b_pin, bemf_a_pin, bemf_b_pin, my_bemf_callback);
}
```

## 3. Steuerung des Motors

Um die Geschwindigkeit und Richtung des Motors zu steuern, verwenden Sie die Funktion `hal_motor_set_pwm()`. Diese Funktion sollte aus Ihrer Hauptschleife oder einer periodischen Aufgabe aufgerufen werden, um den Zustand des Motors zu aktualisieren.

```cpp
void loop() {
  // Motor auf 50 % Tastverhältnis, Vorwärtsrichtung einstellen
  hal_motor_set_pwm(128, true);

  delay(1000);

  // Motor auf 25 % Tastverhältnis, Rückwärtsrichtung einstellen
  hal_motor_set_pwm(64, false);

  delay(1000);
}
```

## 4. Verarbeiten von BEMF-Daten

Die Bibliothek verwendet eine Callback-Funktion, um Ihnen Echtzeit-BEMF-Daten (Back-EMF) bereitzustellen. Sie müssen eine Funktion mit der Signatur `hal_bemf_update_callback_t` implementieren und an `hal_motor_init()` übergeben. Diese Funktion wird aus einem Interrupt-Kontext aufgerufen, wann immer eine neue BEMF-Messung verfügbar ist.

**Wichtig:** Halten Sie den Code innerhalb der Callback-Funktion so kurz und effizient wie möglich, da er in einem Interrupt-Kontext ausgeführt wird. Vermeiden Sie langwierige Operationen oder blockierende Aufrufe.

```cpp
void my_bemf_callback(int raw_bemf_value) {
  // Beispiel: Speichern des neuesten BEMF-Werts in einer flüchtigen Variablen
  // zur Verarbeitung in der Hauptschleife.
  volatile int latest_bemf = raw_bemf_value;
}
```

## 5. Diagnose und Debugging

Für Debugging- und Visualisierungszwecke können Sie mit `hal_motor_get_bemf_buffer()` direkten Zugriff auf den BEMF-ADC-Beispielpuffer erhalten. Dies ist eine erweiterte Funktion und wird für den normalen Betrieb normalerweise nicht benötigt.

```cpp
void debug_bemf_buffer() {
  volatile uint16_t* bemf_buffer;
  int last_write_pos;
  int buffer_size = hal_motor_get_bemf_buffer(&bemf_buffer, &last_write_pos);

  // Sie können jetzt den Inhalt von bemf_buffer überprüfen
}
```

## 6. Plattformspezifika

Dies ist eine Hardware-Abstraktionsschicht (HAL), und die zugrunde liegende Implementierung ist spezifisch für den von Ihnen verwendeten Mikrocontroller. Stellen Sie sicher, dass Sie die richtigen Hardware-Definitionen und die richtige PlatformIO-Umgebung für Ihr Ziel-Board ausgewählt haben.
