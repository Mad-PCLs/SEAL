# SEAL - ISCA 2025 Artifact
In this codebase we combine all three repositories used to evaluate SEAL's integration with HybVIO, VINS-Mono and the HD1K optical flow benchmark.
Each standalone folder contains at its root instructions for:
1. Downloading the required dataset.
2. Building the simulator.
3. Reproducing the results found in Tables 10, 11, and 12 of the SEAL paper.

Each standalone folder has a `docker/` subfolder with the respective `Dockerfile` that will be used to build the docker image.
- There is also a `docker_helper.sh` script that will help with:
    - Building (`./docker_helper.sh build`) the image and
    - Running (`./docker_helper.sh run`) the container.
 
## Dependencies

The artifact can be run on generic x86 Linux machines with Docker installed.

## Cloning the Repository

The repository contains submodules, thus it should be cloned recursively:

```bash
# Git Clone with SSH:
git clone --recursive git@github.com:Mad-PCLs/SEAL.git
# or with HTTPS:
git clone --recursive https://github.com/Mad-PCLs/SEAL.git
```

Or, when downloaded (e.g. as a zip), the repo and submodules could be initialized and updated with:

```bash
git init
git remote add origin git@github.com:Mad-PCLs/SEAL.git
git fetch origin
git reset --hard origin/main
git submodule update --init --recursive
```

## HybVIO (SEAL_HybVIO)
The instructions to follow for reproducing the second column (HybVIO-SEAL results) in Table 10 and Table 11 can be found in its [README.md](./SEAL_HybVIO/README.md). Specifically follow:

- Section: "Setup the Environment"
    1. Build and Run the Docker Container.
    2. Build HybVIO and Dependencies.
    3. Download Required Files.

- Section: "Running Experiments"
    - Sub-section: "Reproduce Tables found in the Paper"

## VINS-Mono (SEAL_VINS_Mono)
The instructions to follow for reproducing the fourth column (VINS-Mono-SEAL) in Table 10 can be found in its [README.md](./SEAL_VINS_Mono/README.md). Specifically follow:

- Section: "Setup the Environment"
    1. Build and Run the Docker Container.
    2. Build SEAL.
    3. Build VINS-Mono with Catkin.
    4. Download Required Files.

- Section: "Running Experiments"
    - Sub-section: "Reproduce Tables found in the Paper"
 
Note: Slight differences between the VINS-Mono results from Table 10 and reproduced experiments are possible due to the framework’s non-determinism, but the numbers should remain similar and do not significantly affect the overall conclusions of the comparison.

## HD1K Benchmark (SEAL_Optical_Flow)
The instructions to follow for reproducing Table 12 can be found in its [README.md](./SEAL_Optical_Flow/README.md). Specifically follow:

- Section: "Setup the Environment"
    1. Build Docker image and run the container.
    2. Download the dataset.
    3. Reproduce results for Table 12.
    4. Parse the results.
