# SEAL Optical Flow

To reproduce the results, follow the steps below:
## 1. Build docker image and run the container
```bash
cd docker/
chmod +x docker_helper.sh

# Build the image
./docker_helper.sh build

# Run the container
./docker_helper.sh run
```

## 2. Download the dataset
While inside the docker container we will first download the dataset `HD1K`.
```bash
cd /home/SEAL_Optical_Flow/scripts/

chmod +x download_hd1k.sh
./download_hd1k.sh
```

# 3. Reproduce results for Table 12
Once the dataset is ready, we can build and run the SEAL optical flow simulator:
```bash
# Go to the [repo_root] directory
cd ..

# Create a build folder and build the simulator
mkdir build
cd build
cmake ..
make

# Run the experiments
./SEAL_Optical_Flow
```

# 4. Parse the results
In order to retrieve the values reported in Table 12 we will parse the results obtained in the previous step using python and print the final output to the console.

```bash
cd ../scripts
python calculate_table_results.py
```

## Python Dependencies:
- Pandas
- NumPy
