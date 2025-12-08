import os
import subprocess
import sys
import matplotlib
matplotlib.use('Agg') # Use non-interactive backend for CI
import matplotlib.pyplot as plt
import numpy as np

def run_simulation(netlist, result_file, output_png, title):
    # Change to simulation directory
    sim_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(sim_dir)

    print(f"Running ngspice for {netlist}...")
    try:
        subprocess.run(["ngspice", "-b", netlist], check=True)
    except FileNotFoundError:
        print("Error: ngspice not found. Please install ngspice.")
        sys.exit(1)
    except subprocess.CalledProcessError as e:
        print(f"Error running ngspice: {e}")
        sys.exit(1)

    if not os.path.exists(result_file):
        print(f"Error: {result_file} not generated.")
        sys.exit(1)

    print(f"Parsing results for {netlist}...")
    data = parse_ngspice_ascii(result_file)

    if not data:
        print("Error: No data parsed.")
        sys.exit(1)

    print(f"Plotting results for {netlist}...")
    plot_results(data, output_png, title)
    print(f"Simulation complete. Graph saved to {output_png}")

def parse_ngspice_ascii(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    values = []
    in_vars = False
    in_vals = False
    var_map = {} # index -> name
    current_point = []

    for line in lines:
        line = line.strip()
        if line.startswith("Variables:"):
            in_vars = True
            continue
        if line.startswith("Values:"):
            in_vars = False
            in_vals = True
            continue

        if in_vars:
            parts = line.split()
            if len(parts) >= 3:
                idx = int(parts[0])
                name = parts[1]
                var_map[idx] = name

        if in_vals:
            if not line: continue
            parts = line.split()

            if len(parts) == 2: # Start of new point: Index Value
                if current_point:
                    values.append(current_point)
                current_point = [float(parts[1])]
            elif len(parts) == 1:
                try:
                    current_point.append(float(parts[0]))
                except ValueError:
                    pass # Ignore non-float lines if any
            elif len(parts) > 2:
                # Handle case where multiple values are on one line (rare but possible)
                if current_point:
                     values.append(current_point)
                current_point = [float(x) for x in parts[1:]]

    if current_point:
        values.append(current_point)

    # Filter out incomplete points if any
    if values:
        expected_len = len(var_map)
        clean_values = [row for row in values if len(row) == expected_len]
        if len(clean_values) < len(values):
            print(f"Warning: Filtered out {len(values) - len(clean_values)} incomplete points.")
        data_arr = np.array(clean_values)
    else:
        data_arr = np.array([])

    result = {}
    for idx, name in var_map.items():
        if idx < data_arr.shape[1]:
            result[name] = data_arr[:, idx]

    return result

def plot_results(data, output_filename, title):
    # Case insensitive lookup
    keys = {k.lower(): k for k in data.keys()}

    if 'time' not in keys:
        print("Error: 'time' variable not found in results.")
        return

    time = data[keys['time']]

    # Try to find voltage and current variables
    voltage_key = keys.get('v(1)', None)
    current_key = keys.get('i(vsense)', None)

    # Try to find BEMF nodes (V4, V5)
    v4_key = keys.get('v(4)', None)
    v5_key = keys.get('v(5)', None)

    if not voltage_key or not current_key:
        print(f"Error: Could not find expected variables 'v(1)' or 'i(vsense)'. Found: {list(data.keys())}")
        return

    voltage = data[voltage_key]
    current = data[current_key]

    bemf = None
    if v4_key and v5_key:
        bemf = data[v4_key] - data[v5_key]

    fig, ax1 = plt.subplots(figsize=(10, 6))

    color_pwm = 'tab:red'
    color_bemf = 'tab:green'

    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Voltage (V)') # Common label for Voltage and BEMF

    # Plot PWM Voltage
    lns1 = ax1.plot(time, voltage, color=color_pwm, label='PWM Voltage', alpha=0.6)

    # Plot BEMF if available
    if bemf is not None:
        lns2 = ax1.plot(time, bemf, color=color_bemf, label='Back EMF', linestyle='--')
    else:
        lns2 = []

    ax1.tick_params(axis='y')
    ax1.grid(True)

    # Plot Current on secondary axis
    ax2 = ax1.twinx()
    color_curr = 'tab:blue'
    ax2.set_ylabel('Current (A)', color=color_curr)
    lns3 = ax2.plot(time, current, color=color_curr, label='Motor Current')
    ax2.tick_params(axis='y', labelcolor=color_curr)

    # Combined legend
    lns = lns1 + lns2 + lns3
    labs = [l.get_label() for l in lns]
    ax1.legend(lns, labs, loc='best')

    plt.title(title)
    fig.tight_layout()
    plt.savefig(output_filename)

if __name__ == "__main__":
    # Run Normal Simulation
    run_simulation(
        "motor.net",
        "simulation_result.txt",
        "simulation_result.png",
        "Motor Simulation: 50% PWM at 20V (Normal)"
    )

    # Run Stalled Simulation
    run_simulation(
        "motor_stalled.net",
        "simulation_stalled_result.txt",
        "simulation_stalled_result.png",
        "Motor Simulation: 50% PWM at 20V (Stalled)"
    )
