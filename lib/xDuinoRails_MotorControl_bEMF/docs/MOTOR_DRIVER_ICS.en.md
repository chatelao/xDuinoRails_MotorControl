# Common Motor Driver ICs

This document describes the pinout and control logic for common motor driver ICs frequently used in model railroading and other hobbyist projects.

## Texas Instruments L293D

**Datasheet:** [https://www.ti.com/lit/ds/symlink/l293.pdf](https://www.ti.com/lit/ds/symlink/l293.pdf)

The L293D is a classic H-bridge driver that can control two motors. It requires separate logic inputs and an enable pin.

| Enable (EN) | Input 1 (IN1) | Input 2 (IN2) | Mode |
|---|---|---|---|
| HIGH | HIGH | LOW | Forward |
| HIGH | LOW | HIGH | Reverse |
| HIGH | LOW | LOW | Brake (Low-Side) |
| HIGH | HIGH | HIGH | Brake (High-Side) |
| LOW | X | X | Coasting |

*Note: `X` means "Don't Care".*

**Current Sensing:** The L293D does not have built-in current sensing. An external shunt resistor is required.

**BEMF Sensing:** BEMF sensing is not a built-in feature. It requires external circuitry to disconnect the motor from the driver and measure the voltage across the motor terminals.

## STMicroelectronics L298N

**Datasheet:** [https://www.st.com/resource/en/datasheet/l298.pdf](https://www.st.com/resource/en/datasheet/l298.pdf)

The L298N is another popular H-bridge driver. Its control is similar to the L293D.

| Enable (EN) | Input 1 (IN1) | Input 2 (IN2) | Mode |
|---|---|---|---|
| HIGH | HIGH | LOW | Forward |
| HIGH | LOW | HIGH | Reverse |
| HIGH | LOW | LOW | Brake |
| HIGH | HIGH | HIGH | Brake |
| LOW | X | X | Coasting |

**Current Sensing:** The L298N has sense pins (Sense A / Sense B) that can be connected to a low-value resistor to ground. The voltage drop across this resistor is proportional to the motor current and can be measured with an ADC.

**BEMF Sensing:** BEMF sensing is not a built-in feature. It requires external circuitry.

## Toshiba TB6612FNG

**Datasheet:** [https://toshiba.semicon-storage.com/us/semiconductor/product/motor-driver-ics/brushed-dc-motor-driver-ics/detail.TB6612FNG.html](https://toshiba.semicon-storage.com/us/semiconductor/product/motor-driver-ics/brushed-dc-motor-driver-ics/detail.TB6612FNG.html)

The TB6612FNG is a modern and efficient MOSFET-based driver. It has a standby pin (STBY) that must be HIGH for the driver to work. Speed is controlled by a separate PWM pin.

| Standby (STBY) | Input 1 (IN1) | Input 2 (IN2) | Mode |
|---|---|---|---|
| HIGH | LOW | HIGH | Forward |
| HIGH | HIGH | LOW | Reverse |
| HIGH | HIGH | HIGH | Brake |
| HIGH | LOW | LOW | Brake |
| LOW | X | X | Coasting / Standby |

*Note: Speed is controlled by a PWM signal on the corresponding PWM pin (PWMA or PWMB).*

**Current Sensing:** The TB6612FNG does not have a dedicated current sense output. An external shunt resistor is required.

**BEMF Sensing:** BEMF sensing is not a built-in feature. It requires external circuitry.

## Texas Instruments DRV8833

**Datasheet:** [https://www.ti.com/lit/ds/symlink/drv8833.pdf](https://www.ti.com/lit/ds/symlink/drv8833.pdf)

The DRV8833 is another popular, modern MOSFET driver, ideal for low-voltage motors. It has a sleep pin and is controlled by two input pins per channel.

| Input 1 (xIN1) | Input 2 (xIN2) | Mode |
|---|---|---|
| HIGH | LOW | Forward |
| LOW | HIGH | Reverse |
| HIGH | HIGH | Brake |
| LOW | LOW | Coasting |

*Note: Speed is controlled by applying a PWM signal to one of the input pins while the other is held at the appropriate logic level for the desired direction.*

**Current Sensing:** The DRV8833 has current limiting pins (xISEN), but these are often grounded on breakout boards, disabling the feature. To measure current, an external shunt resistor is needed.

**BEMF Sensing:** BEMF sensing is not a built-in feature. It requires external circuitry.

## Texas Instruments DRV8874

**Datasheet:** [https://www.ti.com/lit/ds/symlink/drv8874.pdf](https://www.ti.com/lit/ds/symlink/drv8874.pdf)

The DRV8874 is a single-channel H-bridge motor driver with integrated current sensing.

| EN/IN1 | PH/IN2 | Mode |
|---|---|---|
| PWM | HIGH | Forward |
| PWM | LOW | Reverse |
| LOW | X | Coasting (Sleep) |
| HIGH | X | Brake |

*Note: Speed is controlled by the duty cycle of the PWM signal on the EN/IN1 pin.*

**Current Sensing:** The DRV8874 has an ISENSE pin that provides an analog voltage proportional to the motor current.

**BEMF Sensing:** BEMF sensing is not a built-in feature.

## Cytron MDD10A

**Datasheet:** [https://www.cytron.io/p-mdd10a](https://www.cytron.io/p-mdd10a)

The Cytron MDD10A is a high-current driver that uses a simplified control scheme with a direction pin (DIR) and a PWM pin for speed.

| DIR | PWM | Mode |
|---|---|---|
| HIGH | PWM Signal | Forward |
| LOW | PWM Signal | Reverse |
| X | LOW | Brake |

*Note: This driver does not have a pin-controlled coasting mode.*

**Current Sensing:** The MDD10A does not have a dedicated current sense output. An external shunt resistor is required.

**BEMF Sensing:** BEMF sensing is not a built-in feature. It requires external circuitry.

## Pololu High-Power Motor Driver 18v15

**Datasheet:** [https://www.pololu.com/product/755](https://www.pololu.com/product/755)

This is a powerful single-channel driver. It uses a direction pin and a PWM pin.

| PWM | DIR | Mode |
|---|---|---|
| HIGH | LOW | Forward |
| HIGH | HIGH | Reverse |
| LOW | X | Brake |

*Note: This driver does not have a pin-controlled coasting mode.*

**Current Sensing:** This driver does not have built-in current sensing.

**BEMF Sensing:** BEMF sensing is not a built-in feature. It requires external circuitry.

## Pololu High-Power Motor Driver 18v25 CS

**Datasheet:** [https://www.pololu.com/product/1455](https://www.pololu.com/product/1455)

This driver is similar to the 18v15, but includes current sensing and a more advanced control interface that allows for coasting. For typical use, the `PWML` pin can be left disconnected.

| PWMH | DIR | Mode |
|---|---|---|
| PWM Signal | LOW | Forward |
| PWM Signal | HIGH | Reverse |
| LOW | X | Brake |

*Note: To enable coasting, connect the same PWM signal to both PWMH and PWML. In this mode, a LOW signal on the PWM pins will cause the motor to coast instead of brake.*

**Current Sensing:** This driver has a CS pin that outputs an analog voltage proportional to the motor current.

**BEMF Sensing:** BEMF sensing is not a built-in feature. It requires external circuitry.

## Bardeen Micro BDR6133

**Datasheet:** [https://www.lcsc.com/product-detail/C2687793.html](https://www.lcsc.com/product-detail/C2687793.html)

The BDR6133 is a single-channel H-bridge driver.

| INA | INB | Mode |
|---|---|---|
| HIGH | LOW | Forward |
| LOW | HIGH | Reverse |
| HIGH | HIGH | Brake |
| LOW | LOW | Coasting (Stand-by) |

**Current Sensing:** The BDR6133 does not have built-in current sensing.

**BEMF Sensing:** BEMF sensing is not a built-in feature.
