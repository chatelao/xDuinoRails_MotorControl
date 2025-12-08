# Gängige Motortreiber-ICs

Dieses Dokument beschreibt die Pinbelegung und Steuerungslogik für gängige Motortreiber-ICs, die häufig im Modellbau und Hobbyprojekten verwendet werden.

## Texas Instruments L293D

**Datenblatt:** [https://www.ti.com/lit/ds/symlink/l293.pdf](https://www.ti.com/lit/ds/symlink/l293.pdf)

Der L293D ist ein klassischer H-Brücken-Treiber für zwei Motoren.

| Enable (EN) | Input 1 (IN1) | Input 2 (IN2) | Modus |
|---|---|---|---|
| HIGH | HIGH | LOW | Vorwärts |
| HIGH | LOW | HIGH | Rückwärts |
| HIGH | LOW | LOW | Bremse (Low-Side) |
| HIGH | HIGH | HIGH | Bremse (High-Side) |
| LOW | X | X | Leerlauf (Coasting) |

**Strommessung:** Keine integrierte Strommessung.
**BEMF-Messung:** Nicht integriert.

## STMicroelectronics L298N

**Datenblatt:** [https://www.st.com/resource/en/datasheet/l298.pdf](https://www.st.com/resource/en/datasheet/l298.pdf)

Ein weiterer populärer Treiber, ähnlich dem L293D.

| Enable (EN) | Input 1 (IN1) | Input 2 (IN2) | Modus |
|---|---|---|---|
| HIGH | HIGH | LOW | Vorwärts |
| HIGH | LOW | HIGH | Rückwärts |
| HIGH | LOW | LOW | Bremse |
| HIGH | HIGH | HIGH | Bremse |
| LOW | X | X | Leerlauf |

**Strommessung:** Verfügt über Sense-Pins für externe Shunt-Widerstände.

## Toshiba TB6612FNG

**Datenblatt:** [https://toshiba.semicon-storage.com/us/semiconductor/product/motor-driver-ics/brushed-dc-motor-driver-ics/detail.TB6612FNG.html](https://toshiba.semicon-storage.com/us/semiconductor/product/motor-driver-ics/brushed-dc-motor-driver-ics/detail.TB6612FNG.html)

Ein moderner MOSFET-Treiber. Benötigt STBY=HIGH. Geschwindigkeit über PWM-Pin.

| Standby (STBY) | Input 1 (IN1) | Input 2 (IN2) | Modus |
|---|---|---|---|
| HIGH | LOW | HIGH | Vorwärts |
| HIGH | HIGH | LOW | Rückwärts |
| HIGH | HIGH | HIGH | Bremse |
| HIGH | LOW | LOW | Bremse |
| LOW | X | X | Leerlauf / Standby |

## Texas Instruments DRV8833

**Datenblatt:** [https://www.ti.com/lit/ds/symlink/drv8833.pdf](https://www.ti.com/lit/ds/symlink/drv8833.pdf)

Moderner MOSFET-Treiber für niedrige Spannungen.

| Input 1 (xIN1) | Input 2 (xIN2) | Modus |
|---|---|---|
| HIGH | LOW | Vorwärts |
| LOW | HIGH | Rückwärts |
| HIGH | HIGH | Bremse |
| LOW | LOW | Leerlauf |

## Bardeen Micro BDR6133

**Datenblatt:** [https://www.lcsc.com/product-detail/C2687793.html](https://www.lcsc.com/product-detail/C2687793.html)

Einzelkanal H-Brücke.

| INA | INB | Modus |
|---|---|---|
| HIGH | LOW | Vorwärts |
| LOW | HIGH | Rückwärts |
| HIGH | HIGH | Bremse |
| LOW | LOW | Leerlauf (Stand-by) |
