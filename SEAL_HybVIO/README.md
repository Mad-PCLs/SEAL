# SEAL HybVIO

This repository contains the code and scripts for running, evaluating, and analyzing **HybVIO** with SEAL on the EuRoC dataset. It supports accuracy, latency, and power experiments, and provides utilities for dataset preparation and result analysis.

## Setup the Environment

### 1. Build and Run the Docker Container
```bash
cd docker
# Update permissions and build the Docker image
chmod +x docker_helper.sh
./docker_helper.sh build

# Run the Docker container
./docker_helper.sh run
```

### 2. Build HybVIO and Dependencies
Once inside the Docker container:

```bash
# Build 3rd-party dependencies
cd [root-dir-name]/SEAL_HybVIO/HybVIO/3rdparty/mobile-cv-suite/
BUILD_VISUALIZATIONS=OFF ./scripts/build.sh

# Build HybVIO
cd ../..
mkdir build
cd build
cmake -DBUILD_VISUALIZATIONS=OFF -DBUILD_WITH_GPU=OFF -DUSE_SLAM=ON ..
make -j 8
```

### 3. Download Required Files
```bash
# Download the ORB vocabulary
cd ..
./src/slam/download_orb_vocab.sh

# Download and prepare the EuRoC dataset
cd ../scripts
python3 download_euroc.py -type=hybvio
```

## Running Experiments

### Accuracy Experiments
Run HybVIO on sequences and calculate accuracy metrics.

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

## (Optional) Additional Experiments

### Latency Experiments
Measure the latency of HybVIO using its built-in timer.

```bash
# Run all sequences:
./run_hybvio_latency.sh

# Restrict the binary to a single CPU core:
./run_hybvio_latency.sh --restrict
```

### Power Experiments
Simulate power measurements using precomputed optical flow data.

```bash
# Run experiments with optical flow:
./run_hybvio_power.sh

# Run experiments without optical flow (vanilla mode):
./run_hybvio_power.sh --vanilla
```

## Caching Mechanism

To avoid unnecessary reruns, the system uses a caching mechanism:
- Output directories are preserved for each sequence.
- If a script is interrupted, the cached files for that run are automatically cleaned up.
- Specifically for the accuracy experiments, if the `seal_params.yaml` file is modified, the cached files are not deleted, but a warning is issued to alert the user. This ensures that users are aware of potential inconsistencies due to parameter changes.

## Notes

- **Storage Requirements**: The raw EuRoC dataset and its benchmark format require approximately **40GB of storage**. Ensure you have sufficient disk space before downloading and converting the dataset.
- **Ground Truth**: The raw dataset is used for ground truth data, while the benchmark format is used for running experiments.
- **Working Directory**: Ensure you are in the `scripts/` directory when running the scripts.

## Folder Structure

- **`accuracy/`**: Contains scripts for running HybVIO and calculating accuracy metrics.
- **`latency/`**: Contains scripts for measuring and analyzing the latency of HybVIO.
- **`power/`**: Contains scripts for conducting power experiments using precomputed optical flow data.
- **`utils/`**: Contains utility scripts for downloading, converting, and preparing the EuRoC dataset.
