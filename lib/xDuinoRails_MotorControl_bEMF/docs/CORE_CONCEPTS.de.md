# Kernkonzepte

Dieses Dokument erläutert die Kernkonzepte und die Architektur der `xDuinoRails_MotorControl_bEMF`-Bibliothek.

## 1. Hardware-Abstraktionsschicht (HAL)

Die `xDuinoRails_MotorControl_bEMF`-Bibliothek ist als Hardware-Abstraktionsschicht (HAL) konzipiert. Das bedeutet, dass sie eine standardisierte, übergeordnete API für die Motorsteuerung bereitstellt, während die untergeordneten, plattformspezifischen Details der Hardware-Implementierung verborgen werden.

Die Hauptvorteile dieses Ansatzes sind:
- **Portabilität:** Ihr Anwendungscode ist nicht an einen bestimmten Mikrocontroller gebunden. Solange eine HAL-Implementierung für Ihre Zielplattform existiert, sollte Ihr Code mit minimalen Änderungen funktionieren.
- **Einfachheit:** Die HAL bietet eine vereinfachte und konsistente Schnittstelle zu komplexen Hardware-Peripheriegeräten wie Timern, PWM-Generatoren, ADCs und DMA-Controllern.
- **Wartbarkeit:** Der plattformspezifische Code ist sauber von der Anwendungslogik getrennt, was beides einfacher zu warten und zu debuggen macht.

Die öffentliche API der HAL ist in `motor_control_hal.h` definiert. Die Implementierungen für die unterstützten Mikrocontroller (z. B. ESP32, RP2040, STM32) werden in separaten `.cpp`-Dateien bereitgestellt.

## 2. PWM-Motorsteuerung

Die Pulsweitenmodulation (PWM) ist eine Technik zur Steuerung der an ein Gerät, in diesem Fall einen Gleichstrommotor, abgegebenen Leistung. Durch Variieren des Tastverhältnisses eines Rechtecksignals können wir die Motordrehzahl effektiv steuern.

Die Bibliothek verwendet die Hardware-PWM-Peripheriegeräte des Mikrocontrollers, um diese Signale effizient zu erzeugen, ohne CPU-Ressourcen zu verbrauchen. Die Funktion `hal_motor_set_pwm()` ermöglicht es Ihnen, sowohl das Tastverhältnis als auch die Richtung des Motors einzustellen.

## 3. Gegen-EMK-Messung (BEMF)

Wenn sich ein Gleichstrommotor dreht, wirkt er auch als Generator und erzeugt eine Spannung, die als Gegen-EMK (BEMF) bekannt ist. Die Größe dieser BEMF ist direkt proportional zur Drehzahl des Motors. Durch Messen der BEMF können wir auf die Motordrehzahl schließen, ohne einen separaten Sensor zu benötigen, was eine sogenannte "sensorlose" Motorsteuerung ermöglicht.

Um die BEMF zu messen, verwendet die Bibliothek den Analog-Digital-Wandler (ADC) des Mikrocontrollers. Zwei ADC-Kanäle werden verwendet, um die Spannung an den beiden Motorklemmen zu messen. Die Differenz zwischen diesen beiden Messwerten ergibt eine differenzielle BEMF-Messung, die robust gegenüber Gleichtaktstörungen ist.

## 4. DMA-basierte ADC-Abtastung

Um eine leistungsstarke, nicht blockierende BEMF-Messung zu erreichen, nutzt die Bibliothek den Direct Memory Access (DMA)-Controller des Mikrocontrollers.

Der DMA ist so konfiguriert, dass er die ADC-Umwandlungsergebnisse automatisch in einen Ringpuffer im Speicher überträgt. Dies geschieht im Hintergrund, ohne jegliche CPU-Intervention. Sobald ein neues BEMF-Sample im Puffer verfügbar ist, löst die HAL einen Interrupt aus und ruft die vom Benutzer bereitgestellte Callback-Funktion auf.

Diese Architektur stellt sicher, dass die BEMF-Daten mit einer konstanten Rate abgetastet werden und dass die Hauptanwendung nicht blockiert wird, während sie auf ADC-Umwandlungen wartet. Es ist ein hocheffizienter und skalierbarer Ansatz für die Echtzeit-Motorsteuerung.
