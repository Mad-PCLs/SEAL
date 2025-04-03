# VINS-Mono with SEAL Integration

This repository contains the code and scripts for running and evaluating **VINS-Mono** with SEAL on the EuRoC dataset. The package provides tools for accuracy experiments and dataset preparation, with support for caching and parameter validation.

## SEAL Modifications for VINS-Mono Compatibility

The SEAL implementation used in this repository contains specific modifications to maintain compatibility with VINS-Mono's requirements, particularly its dependency on ROS kinetic and OpenCV 3.3.1. These changes are clearly marked in the codebase with the following comment tag:

```
PCL comment: VINS-Mono compatibility change
```

An example of these minor changes can be found in [SparsePyrLKOpticalFlowSealImpl.cpp](./SEAL/src/keypoint_tracking/SparsePyrLKOpticalFlowSealImpl.cpp).
To locate all compatibility changes:
```bash
grep -rn "PCL comment: VINS-Mono compatibility change" ./SEAL/
```

These modifications ensure SEAL maintains full functionality while meeting VINS-Mono's dependency requirements, without impacting the algorithm's core estimation capabilities. The changes are isolated to OpenCV-specific implementations and don't affect the underlying state estimation.

## Setup the Environment

### 1. Build and Run the Docker Container
```bash
# Go to the docker/ directory
cd docker/

# Update permissions and build the Docker image
chmod +x docker_helper.sh
./docker_helper.sh build

# Run the Docker container
./docker_helper.sh run
```
### 2. Build SEAL
SEAL is included inside the VINS-Mono ros packages as a library, so we first have to build it separately.
- The file where all the parameters are defined and loaded in VINS can be found here: [seal_params.yaml](src/VINS-Mono/config/SEALparams/seal_params.yaml).
```bash
# Move inside the SEAL directory
cd SEAL_VINS_Mono/SEAL

# Create a build directory and build SEAL
mkdir build
cd build
cmake ..
make -j 8

# Install SEAL so it can be then found by CMake in VINS.
make install
```

### 3. Build VINS-Mono with Catkin
VINS-Mono has been placed inside the `src` folder as required by ROS/catkin for proper compilation and package discovery.

```bash
# From the root of this repo.
cd /home/SEAL_VINS_Mono
catkin_make

# Source ROS setup script
source ./devel/setup.bash
```

### 4. Download Required Files
```bash
# Move into the [repo_root]/scripts/ directory
cd scripts

# Download the dataset
chmod +x download_euroc_bag.sh
./download_euroc_bag.sh
```

## Running Experiments

### Accuracy Experiments
Run VINS-Mono on sequences and calculate RMSE metrics against ground truth.

```bash
# There are some hardcoded paths for ease of use of the scripts so we first need to move into the [repo_root]/scripts/ directory
cd scripts

# Run all sequences: Skipping sequences that have been cached.
python3 run_vins_accuracy.py

# Run a specific sequence: Skipping sequences that have been cached.
python3 run_vins_accuracy.py --sequence MH_01_easy

# Force rerun of a sequence:
python3 scripts/run_vins_accuracy.py --sequence MH_01_easy --rerun

# Rerun all sequences:
python3 scripts/run_vins_accuracy.py --rerun
```

### Reproduce Tables found in the Paper
While inside the [repo-root]/scripts/ folder run:

```bash
# Accuracy of HybVIO with SEAL, (i.e. the fourth column in Table 10).
python3 table10.py
```

## Caching Mechanism

The system implements intelligent caching to optimize experiment runs:
- Results are stored in sequence-specific output folders.
- Cached runs are reused unless `--rerun` is specified.
- SEAL parameter changes trigger warnings but don't automatically clear cache.
