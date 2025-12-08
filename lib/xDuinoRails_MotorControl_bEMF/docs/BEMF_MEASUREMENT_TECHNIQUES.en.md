# BEMF Measurement and Filtering

Accurate measurement of Back-EMF (BEMF) is crucial for precise, sensorless motor control. This document describes the techniques implemented in the project to extract stable speed information from a noisy raw signal.

The process is divided into two main steps: the basic measurement method and a two-stage software filtering.

## 1. Foundation: Differential Measurement

BEMF is measured as the differential voltage between the two motor terminals (`bemfAPin` and `bemfBPin`). This approach is the foundation for all further processing.

```cpp
// From src/main.cpp
int bemfA = analogRead(bemfAPin);
int bemfB = analogRead(bemfBPin);
int measured_bemf = abs(bemfA - bemfB);
```

**Advantages:**
- **Common-Mode Noise Rejection:** Noise occurring on both lines simultaneously (e.g., induced by PWM switching) is effectively eliminated by differencing.
- **Robust Basis:** Provides a better raw signal than measuring against ground.

## 2. Software Filtering for Signal Smoothing

The differential BEMF signal is still heavily noisy, especially with a 3-pole commutator motor. To obtain a stable signal for pulse counting and PI control, a two-stage filter cascade is used.

### Stage 1: EMA Filter (Exponential Moving Average)

The raw BEMF signal is first passed through a simple but effective EMA low-pass filter. This performs initial smoothing and reduces high-frequency noise spikes.

```cpp
// From src/main.cpp
smoothed_bemf = (EMA_ALPHA * measured_bemf) + ((1.0 - EMA_ALPHA) * smoothed_bemf);
```

- **Purpose:** Attenuation of strong, fast noise.
- **Parameters:** The `EMA_ALPHA` value (`0.21`) determines the smoothing strength. A smaller value smooths more but introduces more signal delay.

### Stage 2: Kalman Filter

The already pre-smoothed value from the EMA filter is then passed to a Kalman Filter. This advanced filter is the core component for signal analysis, as it estimates the signal state (the "true" BEMF) considering the physical properties of the motor.

#### Operation and Logic

The Kalman Filter is ideal for this task because it assumes a system model. Simplified: The filter "knows" that the motor has inertia and its speed (and thus BEMF) cannot change arbitrarily fast.

It works in two phases:
1.  **Prediction:** Based on the last state, the filter predicts the next state. It expects continuity.
2.  **Correction (Update):** It compares the prediction with the new (already EMA-filtered) measurement. If the measurement deviates significantly from the prediction, it is classified as probable noise and weighted lightly. If prediction and measurement are close, the measurement is weighted more heavily.

#### Advantages in the Project
- **Optimal Smoothing:** Provides a mathematically optimal estimate of the BEMF signal, leading to very reliable commutation pulse detection.
- **Low Latency:** Causes less phase shift (signal delay) than a strong EMA or RC filter alone, which is critical for control loop stability.
- **Adaptability:** Performance is adapted to motor characteristics via parameters. In the code, `BEMF_MEA_E` (measurement uncertainty) and `BEMF_Q` (process variance) are preset for a motor with high measurement noise and low process dynamics (inertia).

## Implementation Summary

The current solution combines proven techniques into a robust processing chain:

| Stage | Technique | Purpose |
| :--- | :--- | :--- |
| **1** | **Differential Measurement** | Foundation: Capturing the raw signal with common-mode noise rejection. |
| **2** | **EMA Filter** | Initial Smoothing: Attenuating high-frequency noise spikes in the raw signal. |
| **3** | **Kalman Filter** | Final Estimation: Optimal, adaptive signal filtering based on a system model. |

This cascade ensures that reliable speed information can be extracted for the PI controller from the highly noisy signal of a simple brushed motor.
