import argparse
from accuracy.VINSSequenceRunner import VINSSequenceRunner

# Configuration constants
DATA_FOLDER = "../data/bag/"
OUTPUT_ROOT_FOLDER = "../output_accuracy/"
SEAL_PARAMS_PATH = "../src/VINS-Mono/config/SEALparams/seal_params.yaml"
VINS_PACKAGE = "vins_estimator"
VINS_LAUNCH_FILE = "euroc.launch"
DEFAULT_OUTPUT_FOLDER = "/home/shaozu/output/"

SEQUENCE_NAMES = [
    "MH_01_easy",
    "MH_02_easy",
    "MH_03_medium",
    "MH_04_difficult",
    "MH_05_difficult",
    "V1_01_easy",
    "V1_02_medium",
    "V1_03_difficult",
    "V2_01_easy",
    "V2_02_medium",
    "V2_03_difficult"
]

if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(
        description="Run VINS-Mono on sequences and calculate RMSE metrics."
    )
    parser.add_argument(
        "--sequence",
        type=str,
        default=None,
        help="Name of the sequence to run. If not provided, run all sequences.",
    )
    parser.add_argument(
        "--rerun",
        action="store_true",
        help="Delete existing output folders and rerun the sequences.",
    )
    args = parser.parse_args()

    # Initialize the sequence runner
    runner = VINSSequenceRunner(
        data_folder = DATA_FOLDER,
        output_root_folder = OUTPUT_ROOT_FOLDER,
        seal_params_path = SEAL_PARAMS_PATH,
        vins_package = VINS_PACKAGE,
        vins_launch_file = VINS_LAUNCH_FILE,
        default_output_folder = DEFAULT_OUTPUT_FOLDER,
        ground_truth_root_folder="../src/VINS-Mono/benchmark_publisher/config",
        setup_script_path="../devel/setup.bash"
    )

    # Determine which sequences to run
    sequence_names = [args.sequence] if args.sequence else SEQUENCE_NAMES
    
    # Run the selected sequences
    runner.run_sequences(
        sequence_names=sequence_names,
        rerun=args.rerun
    )
