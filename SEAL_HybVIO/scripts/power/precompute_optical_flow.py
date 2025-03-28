# Script used by run_hybvio_power.sh
# It precomputes and stores the optical flow for all EuRoC maps.
import os

from .PowerExperiment import PowerExperiment

# Paths
HYBVIO_ROOT_FOLDER = "../HybVIO"
HYBVIO_EXECUTABLE = os.path.join(HYBVIO_ROOT_FOLDER, "build", "main")

DATASET_ROOT_FOLDER = "../data/benchmark/"
ORB_VOCABULARY_PATH = os.path.join(HYBVIO_ROOT_FOLDER, "data", "orb_vocab.dbow2")

# Program variables
SEQUENCE_NAMES = [
    "euroc-mh-01-easy",
    "euroc-mh-02-easy",
    "euroc-mh-03-medium",
    "euroc-mh-04-difficult",
    "euroc-mh-05-difficult",
    "euroc-v1-01-easy",
    "euroc-v1-02-medium",
    "euroc-v1-03-difficult",
    "euroc-v2-01-easy",
    "euroc-v2-02-medium",
    "euroc-v2-03-difficult"
]
SEAL_POWER_EMULATOR_ROOT_FOLDER = os.path.join(HYBVIO_ROOT_FOLDER, "data/pcl_serial_storage/optical_flow")

if __name__ == "__main__":
    # Create experiments and automatically compute and store optical flow.
    print("Running experiments with -sealPowerEmulatorMode=store...")
    for sequence_name in SEQUENCE_NAMES:
        exper = PowerExperiment(
            id=0,
            executable=HYBVIO_EXECUTABLE,
            dataset_root_folder=DATASET_ROOT_FOLDER,
            sequence_name=sequence_name,
            orb_vocabulary_path=ORB_VOCABULARY_PATH,
            seal_power_emulator_storage_path=SEAL_POWER_EMULATOR_ROOT_FOLDER,
            seal_power_emulator_mode="store"
        )

        if exper.is_optFlow_cereal_available():
            print(f"  Pre-computed optical flow for {sequence_name} found. Continue..")
            continue

        print(f"  Started storing optical flow for: {sequence_name}")
        exper.run_me()
        print(f"    Done!")

    print("DONE: Optical flow for all maps serialized!")
