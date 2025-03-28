# Accuracy Metrics for HybVIO

This folder contains the scripts and logic for running HybVIO on sequences and calculating accuracy metrics. The primary goal is to automate the execution of HybVIO, manage output directories, and compute metrics such as RMSE. The metric calculation logic is based on the original implementation from the [vio_benchmark](https://github.com/AaltoML/vio_benchmark/tree/main/hybvio_runner) repository, which was developed by maintainers of the [HybVIO](https://github.com/SpectacularAI/HybVIO) repository.

## Files

### 1. **`SequenceRunner.py`**
Manages the execution of HybVIO on sequences and handles output directories.

- **Key Features**:
  - Preserves output directories for each sequence to avoid unnecessary reruns.
  - Compares the `seal_params.yaml` file used in previous executions with the current one. If they differ, a warning is issued to alert the user.
  - Ensures that experiments are only rerun when necessary, saving time and computational resources.

### 2. **`OriginalHybVIOMetricsCalculator.py`**
Contains the original logic for calculating accuracy metrics for HybVIO.

- **Purpose**:
  - Implements the metric calculation logic from the [vio_benchmark](https://github.com/AaltoML/vio_benchmark/tree/main/hybvio_runner) repository.
  - This is the recommended tool for calculating metrics, as it is maintained by the original developers of HybVIO.
  - Computes metrics such as RMSE for evaluating trajectory accuracy.

### 3. **`EvoMetricsCalculator.py`**
Custom functions for calculating metrics using the [evo tool](https://github.com/MichaelGrupp/evo).

- **Purpose**:
  - Provided for completeness and alternative evaluation.
  - Loads the estimated trajectory (from a `jsonl` file) and the ground truth (from a EuRoC-format `csv` file) into dataframes of the same format (TUM).
  - Uses the `evo` tool to compute the Root Mean Square Error (RMSE) between the estimated trajectory and the ground truth.

### 4. **`HybVIOExecutor.py`**
Handles the execution of the HybVIO binary and manages input/output files.

- **Key Features**:
  - Manages the execution of the HybVIO binary, ensuring that input files are correctly passed and output files are saved in the appropriate directories.
  - Handles errors and interruptions gracefully, ensuring that partial results are not lost.

## Usage

The `run_hybvio_accuracy.py` script in the `scripts/` directory is the main entry point for running HybVIO and calculating accuracy metrics. It uses the classes in this folder to automate the process.

### Script: `run_hybvio_accuracy.py`
It is located in `[repo_root_folder]/scripts/run_hybvio_accuracy.py`. In order to execute it make sure you are in the scripts folder.

#### Description
This script runs HybVIO on one or more sequences and calculates accuracy metrics. It supports the following arguments:

- `--sequence`: Name of the sequence to run. If not provided, all sequences are run.
- `--rerun`: If provided, deletes existing output folders and reruns the sequences.

#### Example Usage
0. Move to the correct directory:
   ```bash
   cd scripts
   ```

1. Run HybVIO on a specific sequence:
   ```bash
   python run_hybvio_accuracy.py --sequence euroc-mh-01-easy
   ```

2. Run HybVIO on all sequences:
   ```bash
   python run_hybvio_accuracy.py
   ```

3. Rerun a sequence and overwrite existing results:
   ```bash
   python run_hybvio_accuracy.py --sequence euroc-mh-01-easy --rerun
   ```

#### Default Configuration
The script uses the following default paths:
- `BENCHMARK_FOLDER`: `../data/benchmark/`
- `RAW_DATA_FOLDER`: `../data/raw/`
- `OUTPUT_ROOT_FOLDER`: `../output_accuracy/`
- `BINARY_PATH`: `../HybVIO/build/main`
- `ORB_VOCAB_PATH`: `../HybVIO/data/orb_vocab.dbow2`
- `SEAL_PARAMS_PATH`: `../HybVIO/SEAL/seal_params.yaml`

## Caching Mechanism

To avoid unnecessary reruns, the system uses a caching mechanism:
- Output directories are preserved for each sequence.
- If a script is interrupted, the cached files for that run are automatically cleaned up.
- If the `seal_params.yaml` file is modified, the cached files are not deleted, but a warning is issued to alert the user. This ensures that users are aware of potential inconsistencies due to parameter changes.

## Notes

- **Recommended Metric Calculator**: Use the `OriginalHybVIOMetricsCalculator` for metric calculations, as it is based on the original implementation from the [vio_benchmark](https://github.com/AaltoML/vio_benchmark/tree/main/hybvio_runner) repository.
- **Parameter Changes**: Always check the warnings about parameter mismatches. If the `seal_params.yaml` file has been modified, consider rerunning the experiments to ensure consistent results.
- **Evo Tool**: The `evo` tool is provided for completeness but is not the recommended method for calculating metrics.
- **Working Directory**: Ensure you are in the `scripts/` directory when running the scripts.
