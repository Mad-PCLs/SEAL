# Scripts Directory

This directory contains scripts and utilities for running, evaluating, and analyzing **VINS-Mono** on the EuRoC dataset. Below is an overview of the contents:

## Main Components

- **`accuracy/`**: Contains the core logic for running VINS-Mono on sequences and calculating accuracy metrics.
  - `VINSMetricsCalculator.py`: Computes RMSE between estimated and ground truth trajectories
  - `VINSMonoExecutor.py`: Handles ROS execution and process management
  - `VINSSequenceRunner.py`: Coordinates sequence execution with caching and parameter validation

- **`download_euroc_bag.sh`**: Downloads EuRoC rosbag datasets
- **`run_vins_accuracy.py`**: Main script for running accuracy experiments

## Main Scripts

### 1. **`download_euroc_bag.sh`**
Downloads EuRoC dataset rosbags for VINS-Mono experiments.

#### Usage
```bash
# Download all sequences:
./download_euroc_bag.sh
```

#### Key Features
- Supports both full dataset and selective downloads
- Verifies download integrity
- Organizes bags in standard directory structure

### 2. **`run_vins_accuracy.py`**
Runs VINS-Mono on the EuRoC sequences and calculates RMSE metrics.

#### Usage
```bash
# Run all sequences:
python3 run_vins_accuracy.py

# Run a specific sequence:
python3 run_vins_accuracy.py --sequence MH_01_easy

# Force rerun of a sequence (ignore cache):
python3 run_vins_accuracy.py --sequence MH_01_easy --rerun

# Rerun all sequences:
python3 run_vins_accuracy.py --rerun
```

### 3. **`table10.py`**
Runs VINS-Mono on the EuRoC sequences and calculates RMSE metrics,
at the end it compiles all results into Table 10 found in the paper.

#### Usage
```bash
# Run all sequences:
python3 table10.py
```

## Caching Mechanism

The system implements intelligent caching:
- Results are stored in sequence-specific output folders
- Cached runs are reused unless `--rerun` is specified
- SEAL parameter changes trigger warnings but don't automatically clear cache
- Clean handling of interrupted runs

## Notes

1. **First Time Setup**:
   - Run `./download_euroc_bag.sh` to get the required datasets

2. **Running Experiments**:
   - Execute scripts from the `scripts/` directory
   - ROS master processes are automatically managed

3. **Output Files**:
   - Each sequence generates:
     - `vins_output/` with trajectory estimates
     - `execution_info.json` with run metadata
     - `vins_stdout.txt` with console output
