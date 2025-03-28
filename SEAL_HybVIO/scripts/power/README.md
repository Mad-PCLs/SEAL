# Power Measurement for HybVIO

This folder contains the scripts and tools for conducting power experiments for the **HybVIO** system. The experiments simulate power measurements by leveraging precomputed optical flow data. These experiments do not perform actual power measurements but are designed to work alongside external power measurement tools (e.g., a Raspberry Pi 4 and an external power meter).

## Files

### 1. **`PowerExperiment.py`**
Defines the `PowerExperiment` class, which encapsulates the logic for running power experiments.

- **Key Features**:
  - Checks if precomputed optical flow data is available and computes it if necessary.
  - Handles errors and interruptions by cleaning up sequence-specific folders.
  - Executes the HybVIO binary with appropriate arguments and captures output for validation.

### 2. **`precompute_optical_flow.py`**
Precomputes and stores optical flow data for all EuRoC datasets.

- **Purpose**:
  - Ensures that optical flow data is available for power experiments.
  - Skips sequences with precomputed data to save time.

### 3. **`run_hybvio_power.sh`**
A bash script that automates the execution of power experiments.

- **Purpose**:
  - Runs experiments for all sequences in the EuRoC dataset.
  - Stores stdout and estimated trajectories in separate folders.
  - Supports two modes: **default mode** (with optical flow) and **vanilla mode** (without optical flow).

## Usage

### 1. Precompute Optical Flow (Optional for Vanilla Mode)
Before running power experiments, ensure that optical flow data is precomputed:

```bash
# From the scripts/ directory
python3 -m power.precompute_optical_flow
```

### 2. Run Power Experiments
Execute the `run_hybvio_power.sh` script from the `scripts/` directory:

- **Default Mode** (with optical flow):
  ```bash
  # From the scripts/ directory
  ./run_hybvio_power.sh
  ```

- **Vanilla Mode** (without optical flow):
  ```bash
  # From the scripts/ directory
  ./run_hybvio_power.sh --vanilla
  ```

## How It Works

1. **Precompute Optical Flow**:
   - The `precompute_optical_flow.py` script computes and stores optical flow data for all EuRoC datasets.

2. **Run Experiments**:
   - The `run_hybvio_power.sh` script executes the HybVIO binary for each dataset sequence and stores the raw output in the `output_power` folder.

3. **User Interaction**:
   - The script prompts the user to confirm before starting the experiment for each sequence.
   - After each sequence, the user must press a key to continue to the next sequence.

4. **Output Management**:
   - The script stores stdout and estimated trajectories in separate folders for each sequence.

## Notes

- **No Actual Power Measurements**: The scripts in this folder do not perform actual power measurements. The power measurements reported in the paper were taken using an external power meter and a Raspberry Pi 4.
- **Optical Flow Precomputation**: Ensure that optical flow data is precomputed using `precompute_optical_flow.py` before running the experiments (unless using vanilla mode).
- **Caching Mechanism**: The system avoids duplicate runs by checking if output files already exist. If they do, the script skips the corresponding experiment.
- **Interrupt Handling**: If the script is interrupted, it cleans up partially generated files to ensure consistency.
- **Working Directory**: Ensure you are in the `scripts/` directory when running the scripts.
