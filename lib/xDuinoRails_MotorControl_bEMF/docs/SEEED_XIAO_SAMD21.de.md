# Seeed Studio XIAO SAMD21

Dieses Dokument beschreibt die Unterstützung für den Seeed Studio XIAO SAMD21.

## Überblick
Der XIAO SAMD21 ist ein winziges Board basierend auf dem Atmel SAMD21G18 (ARM Cortex-M0+).

## Konfiguration
Die HAL-Implementierung nutzt die Timer und den ADC des SAMD21. Aufgrund von Hardware-Einschränkungen im Vergleich zum RP2040 können einige erweiterte Funktionen unterschiedliche Leistungsmerkmale aufweisen.
