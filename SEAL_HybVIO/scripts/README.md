# Scripts Directory

This directory contains scripts and utilities for running, evaluating, and analyzing **HybVIO** on the EuRoC dataset. Below is an overview of the subfolders and scripts:

- **`accuracy/`**: Contains the main logic for running HybVIO on sequences and calculating accuracy metrics.
  - See [accuracy/README.md](accuracy/README.md) for details.

- **`latency/`**: Contains scripts and tools for measuring and analyzing the latency of the HybVIO system.
  - See [latency/README.md](latency/README.md) for details.

- **`power/`**: Contains scripts for conducting power experiments using precomputed optical flow data.
  - See [power/README.md](power/README.md) for details.

- **`utils/`**: Contains utility scripts for downloading, converting, and preparing the EuRoC dataset.
  - See [utils/README.md](utils/README.md) for details.

## Main Scripts

### 1. **`run_hybvio_accuracy.py`**
Runs HybVIO on specified sequences and calculates accuracy metrics.

#### Usage
```bash
# Run all sequences:
python3 run_hybvio_accuracy.py

# Run a specific sequence:
python3 run_hybvio_accuracy.py --sequence euroc-mh-01-easy

# Rerun a specific sequence:
python3 run_hybvio_accuracy.py --sequence euroc-mh-01-easy --rerun

# Rerun all sequences:
python3 run_hybvio_accuracy.py --rerun
```

### Reproduce Tables found in the Paper
While inside the `[repo-root]/scripts/` folder run:
```bash
# Accuracy of HybVIO with SEAL, (i.e. the second column in Table 10).
python3 table10.py

# Edge threshold sweep found in Table 11.
python3 table11.py
```

#### Key Features
- Preserves output directories to avoid unnecessary reruns.
- Warns the user if the `seal_params.yaml` file used in a previous execution does not match the current one.

### 2. **`run_hybvio_latency.sh`**
Automates the execution of the HybVIO binary across multiple sequences to measure latency.

#### Usage
```bash
# Run all sequences:
./run_hybvio_latency.sh

# Restrict the binary to a single CPU core:
./run_hybvio_latency.sh --restrict
```

#### Key Features
- Iterates over a predefined list of dataset sequences.
- Skips sequences if output files already exist.
- Supports restricting the binary to a single CPU core using the `--restrict` flag.
- Automatically parses raw outputs to compute latency averages.

### 3. **`run_hybvio_power.sh`**
Automates power experiments using precomputed optical flow data.

#### Usage
```bash
# Run experiments with optical flow:
./run_hybvio_power.sh

# Run experiments without optical flow (vanilla mode):
./run_hybvio_power.sh --vanilla
```

#### Key Features
- Runs experiments for all sequences in the EuRoC dataset.
- Prompts the user to confirm before starting each sequence.
- Supports two modes: **default mode** (with optical flow) and **vanilla mode** (without optical flow).

---

### 4. **`download_euroc.py`**
Downloads and prepares the EuRoC dataset for use with HybVIO.

#### Usage
```bash
# Download and prepare the EuRoC dataset:
python3 download_euroc.py -type hybvio
```

#### Key Features
- Downloads and extracts the EuRoC dataset from the official source.
- Converts the dataset to the benchmark format required by HybVIO.
- Ensures clean handling of partially downloaded or extracted datasets.

## Caching Mechanism

To avoid unnecessary reruns, the system uses a caching mechanism:
- Output directories are preserved for each sequence.
- If a script is interrupted, the cached files for that run are automatically cleaned up.
- Specifically for the accuracy experiments, if the `seal_params.yaml` file is modified, the cached files are not deleted, but a warning is issued to alert the user. This ensures that users are aware of potential inconsistencies due to parameter changes.

## Notes

- **Storage Requirements**: The raw EuRoC dataset and its benchmark format require approximately **40GB of storage**. Ensure you have sufficient disk space before downloading and converting the dataset.
- **Working Directory**: Ensure you are in the `scripts/` directory when running the scripts.
- **Ground Truth**: The raw dataset is used for ground truth data, while the benchmark format is used for running HybVIO experiments.
