# Adafruit Feather nRF52840 Express

Dieses Dokument beschreibt die Unterstützung für den Adafruit Feather nRF52840.

## Überblick
Der Feather nRF52840 verfügt über ein Nordic nRF52840 SoC (ARM Cortex-M4F) mit Bluetooth LE Unterstützung.

## Konfiguration
Der HAL nutzt das PPI-System (Programmable Peripheral Interconnect) des nRF52, um Timer und ADC für eine latenzarme Steuerung ohne CPU-Eingriff zu verbinden.
