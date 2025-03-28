# Latency Measurement for HybVIO

This folder contains the scripts and tools for analyzing the latency of the **HybVIO** system using its built-in timer. The tool parses raw output files from the HybVIO binary, extracts timing measurements, and computes averages across multiple runs and sequences.

## Files

### 1. **`VIOFrontendMeasurements.py`**
Stores timing measurements for the **VIO frontend**, including:
- `findKeypoints`: Time taken for keypoint detection.
- `computeOpticalFlow`: Time taken for keypoint tracking (optical flow computation).

### 2. **`VIOBackendMeasurements.py`**
Stores timing measurements for the **VIO backend**, including:
- `doRansac2`: Time taken for RANSAC with 2 points.
- `doRansac5`: Time taken for RANSAC with 5 points.
- `trackerVisualUpdate`: Time taken for visual updates in the tracker.
- `KFpredict`: Time taken for Kalman Filter prediction.

These dataclasses support **addition** and **division** operations, enabling easy accumulation and averaging of measurements.

### 3. **`VIOExperimentFactory.py`**
Responsible for parsing raw output files and creating `VIOExperiment` instances. It contains:
- `parse_output_times`: Extracts timing measurements from the raw output file.
- `create_from_file`: Creates a `VIOExperiment` instance from a file path.

### 4. **`parse_output_latency.py`**
The main script that processes all raw output files, computes averages for each sequence, and calculates overall averages across all sequences. It uses the dataclasses and factory to:
- Accumulate measurements across multiple runs.
- Compute per-sequence averages.
- Compute overall averages across all sequences.

## Usage

### Running Experiments
To run the experiments, execute the `run_hybvio_latency.sh` script from the `scripts/` directory:

```bash
# From the scripts/ directory
./run_hybvio_latency.sh
```

Use the `--restrict` flag to restrict the HybVIO binary to a single CPU core:

```bash
# From the scripts/ directory
./run_hybvio_latency.sh --restrict
```

### Parsing Results
After running the experiments, the `parse_output_latency.py` script is automatically executed to process the raw outputs. If you want to manually parse the results, you can run:

```bash
# From the scripts/ directory
python3 -m latency.parse_output_latency
```

## How It Works

1. **Run Experiments**:
   - The `run_hybvio_latency.sh` script executes the HybVIO binary for each dataset sequence and stores the raw output in the `output_latency` folder.

2. **Parse Raw Outputs**:
   - The `parse_output_latency.py` script processes the raw output files:
     - Extracts timing measurements using the `VIOExperimentFactory`.
     - Accumulates measurements across multiple runs.
     - Computes per-sequence and overall averages.

3. **Output Results**:
   - The script prints the following metrics for each sequence and overall:
     - **Keypoint Detection (KD)**
     - **Keypoint Tracking (KT)**
     - **RANSAC**
     - **Backend (BE)**

## Notes

- **Caching Mechanism**: The system avoids duplicate runs by checking if output files already exist. If they do, the script skips the corresponding experiment.
- **Interrupt Handling**: If the script is interrupted, it cleans up partially generated files to ensure consistency.
- **Single-Core Execution**: Use the `--restrict` flag to restrict HybVIO to a single CPU core for more controlled latency measurements.
- **Working Directory**: Ensure you are in the `scripts/` directory when running the scripts.
