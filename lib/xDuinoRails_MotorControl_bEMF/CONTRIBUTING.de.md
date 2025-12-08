# Mitwirken

Vielen Dank für Ihr Interesse, zum `xDuinoRails_MotorControl_bEMF`-Projekt beizutragen. Dieses Dokument beschreibt die Richtlinien und Best Practices für Beiträge.

## Wie man beitragen kann

Wir freuen uns über Beiträge in Form von Fehlerberichten, Funktionswünschen und Pull-Requests.

- **Fehlerberichte:** Wenn Sie einen Fehler finden, eröffnen Sie bitte ein Issue auf GitHub und geben Sie eine detaillierte Beschreibung des Problems an, einschließlich der von Ihnen verwendeten Hardware und der Schritte zur Reproduktion des Problems.
- **Funktionswünsche:** Wenn Sie eine Idee für eine neue Funktion haben, eröffnen Sie bitte ein Issue, um sie zu diskutieren. Dies ermöglicht es uns, uns vor Beginn der Programmierung über das Design und die Implementierung abzustimmen.
- **Pull-Requests:** Wir freuen uns über Pull-Requests für Fehlerbehebungen und neue Funktionen. Bevor Sie einen Pull-Request einreichen, stellen Sie bitte sicher, dass Ihr Code den Richtlinien in diesem Dokument entspricht.

## Entwicklungsrichtlinien

### Codestil

- **Namenskonventionen:** Verwenden Sie `snake_case` für Datei- und Variablennamen.
- **Kommentare:** Fügen Sie Kommentare hinzu, um komplexe Logik oder magische Zahlen zu erklären.
- **Variablenausrichtung:** Richten Sie Variablenzuweisungen am Gleichheitszeichen (`=`) aus und richten Sie Zahlen rechtsbündig aus.

### Dokumentation

- Die gesamte Dokumentation befindet sich im `docs`-Verzeichnis.
- Verwenden Sie `UPPER_SNAKE_CASE` für alle `.md`-Dateinamen.
- Wenn Sie Änderungen vornehmen, aktualisieren Sie bitte die relevanten Dokumentationsdateien (`HOW_TO_USE.md`, `USER_MANUAL.md`, `CORE_CONCEPTS.md`, `DEVELOPER_REFERENCE.md`, `TECHNICAL_DEBTS.md`).

### Testen

- Fügen Sie zu jedem Testfall einen Kommentar hinzu, der das Ziel und die durchgeführten Schritte zusammenfasst.
- Verweisen Sie für jeden Testfall auf die zugrunde liegende Spezifikation oder Dokumentation.

## Außerhalb des Geltungsbereichs

Die folgenden Themen liegen außerhalb des Geltungsbereichs dieses Projekts:
- Digitale Modellbahnprotokolle (z. B. DCC, RailCom, ACC).
- Benutzeroberflächen und Kommunikation (z. B. CLI, OLED-Displays, Webserver, BLE).

Wir freuen uns auf Ihre Beiträge!
