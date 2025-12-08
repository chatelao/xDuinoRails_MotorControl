# Motor Simulation

This directory contains an LTSpice-compatible simulation of a brushed DC motor driven by a PWM signal.

## Files

- `motor.net`: The SPICE netlist defining the motor model and simulation parameters.
- `run_sim.py`: A Python script to run the simulation using `ngspice` and plot the results.
- `simulation_result.png`: The output graph of the simulation.

## Running the Simulation

To run the simulation locally, you need `ngspice` and `python3` with `matplotlib` and `numpy`.

```bash
python run_sim.py
```

The simulation models a motor with:
- 20V Supply
- 50% Duty Cycle PWM at 20kHz
- Armature Resistance: 12 Ohm
- Armature Inductance: 2 mH
- Back EMF Constant: 0.01 V/(rad/s)
- Torque Constant: 0.01 N*m/A
- Rotor Inertia: 1e-6 kg*m^2
- Friction: 1e-5 N*m*s/rad
