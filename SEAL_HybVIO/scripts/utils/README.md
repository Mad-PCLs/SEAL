# Utilities for EuRoC Dataset Preparation

This folder contains utility scripts for downloading, extracting, and converting the EuRoC dataset into the format required by **HybVIO**. The scripts also handle type definitions for JSONL files used in the project.

## Files

### 1. **`BenchmarkConverter.py`**
Handles the conversion of the raw EuRoC dataset into the benchmark format required by HybVIO. This script is based on the [vio_benchmark](https://github.com/AaltoML/vio_benchmark/blob/main/convert/euroc_to_benchmark.py) repository.

- **Key Features**:
  - Converts raw EuRoC dataset files (ground truth, IMU data, and camera data) into the benchmark format.
  - Processes timestamps, camera parameters, and frames.
  - Generates JSONL files, videos, and additional metadata files (`info.json` and `parameters.txt`).
  - Handles partial conversions and cleans up in case of errors or interruptions.

### 2. **`EurocDatasetManager.py`**
Manages the EuRoC dataset, including downloading, extracting, and preparing the data for use with HybVIO.

- **Key Features**:
  - Downloads and extracts the EuRoC dataset from the official source.
  - Prepares the dataset for either VINS or HybVIO by converting it to the required format.
  - Ensures that partially downloaded or extracted datasets are cleaned up in case of errors or interruptions.

### 3. **`jsonl_types.py`**
Defines types used by `BenchmarkConverter.py` to structure the data for JSONL files.

- **Key Features**:
  - Defines typed data structures for vision kind (`VisionKind`), camera parameters (`CameraParameters`), frames (`Frame`), positions (`Position`), orientations (`Orientation`), ground truth (`GroundTruth`), sensor data (`SensorData`), and dataset entries (`DatasetEntry`).
  - Ensures type safety and consistency when working with JSONL files.

## Usage

### Download and Prepare the EuRoC Dataset
To download and prepare the EuRoC dataset, use the `download_euroc.py` script located in the `scripts/` directory:

```bash
# From the scripts/ directory
python3 download_euroc.py -type hybvio
```

- **`-type`**: Specify the type of data to prepare (`vins` or `hybvio`). Use `hybvio` to convert the dataset to the benchmark format required by HybVIO.

## Notes

- **Storage Requirements**: The raw EuRoC dataset and its benchmark format require approximately **40GB of storage**. Ensure you have sufficient disk space before downloading and converting the dataset.
- **Ground Truth**: The raw dataset is used for ground truth data, while the benchmark format is used for running experiments.
- **Working Directory**: Ensure you are in the `scripts/` directory when running the `download_euroc.py` script.
