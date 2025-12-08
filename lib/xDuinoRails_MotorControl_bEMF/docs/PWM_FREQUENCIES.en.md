# PWM Control and Frequency Selection

Pulse Width Modulation (PWM) is the core technology for controlling motor power. This document describes the current implementation and the implications of the chosen PWM frequency.

## Current Implementation: Hardware-Accelerated PWM

The project uses a hardware timer of the RP2040 to implement non-blocking PWM control. This is a significant departure from simple software-based `analogWrite()` implementation.

- **PWM Frequency:** The base frequency is fixed at **1 kHz**.
- **Timer Logic:** A repeating timer triggers the start of each PWM cycle (the ON phase). Within this timer, a second, one-shot alarm is scheduled for the end of the ON phase. The callback function of this alarm then performs the BEMF measurement.

**Advantages of this Implementation:**
- **Non-blocking:** The CPU is not blocked by `delay()` calls and is available for other tasks (state logic, communication).
- **Precise Timing:** The timing of PWM edges and BEMF measurement is hardware-accurate and independent of main processor load.
- **Efficient:** Significantly reduces CPU load compared to software PWM.

## Analysis of Current PWM Frequency (1 kHz)

The choice of 1 kHz is a conscious compromise for the initial implementation.

### Advantages at 1 kHz

- **Low Switching Losses:** The BDR6133 motor driver switches relatively infrequently, minimizing thermal stress and increasing efficiency.
- **Large Time Window for BEMF Measurement:** A 1 kHz cycle lasts 1000 microseconds (`µs`). Even at 90% PWM duty cycle, 100 µs remain for bridge turn-off, voltage settling, and ADC measurement. This is sufficient time for the currently implemented `delayMicroseconds(100)` pause.

### Disadvantages at 1 kHz

- **Audible Operation Noise:** Motors tend to generate a faint whistle or hum at frequencies in the human hearing range (up to approx. 18 kHz). This is the case at 1 kHz and often undesirable for high-quality model railroad applications.
- **Torque Ripple:** The current through the motor windings is not fully smoothed, which can lead to slightly uneven running, especially at very low speeds.

## Outlook: Transition to Higher PWM Frequency

For future versions, increasing the PWM frequency into the ultrasonic range (e.g., > 18 kHz) is desirable to make motor operation silent and further smooth motor running. However, such a transition is not trivial and has several consequences:

1.  **Severely Reduced Time Window:** At 20 kHz, a cycle lasts only 50 µs. The time window for BEMF measurement becomes extremely short. The current `delayMicroseconds(100)` pause would be longer than the entire PWM cycle and is **no longer possible**. Timing would need to be completely revised to work with a few microseconds.
2.  **Increased Switching Losses:** The motor driver would switch 20 times more frequently, leading to significantly higher heat generation. It would need to be evaluated if the driver can handle this without additional cooling.
3.  **Faster ADC Measurement:** ADC conversion would need to occur quickly and precisely within the short time window.

## Conclusion

The current 1 kHz PWM realized via hardware timers is a robust and efficient foundation. It offers a stable system with plenty of time for reliable BEMF measurement. A future frequency increase for noise reduction is a logical next step but requires significant revision of BEMF measurement timing and careful analysis of thermal effects on the motor driver.
