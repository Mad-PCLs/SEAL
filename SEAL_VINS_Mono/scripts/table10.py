import os
import numpy as np
from tabulate import tabulate
from accuracy.VINSSequenceRunner import VINSSequenceRunner
from accuracy.VINSMetricsCalculator import VINSMetricsCalculator

# Configuration constants
DATA_FOLDER = "../data/bag/"
OUTPUT_ROOT_FOLDER = "../output_accuracy/table10/"
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
    # Initialize the sequence runner
    runner = VINSSequenceRunner(
        data_folder=DATA_FOLDER,
        output_root_folder=OUTPUT_ROOT_FOLDER,
        seal_params_path=SEAL_PARAMS_PATH,
        vins_package=VINS_PACKAGE,
        vins_launch_file=VINS_LAUNCH_FILE,
        default_output_folder=DEFAULT_OUTPUT_FOLDER,
        ground_truth_root_folder="../src/VINS-Mono/benchmark_publisher/config",
        setup_script_path="../devel/setup.bash",
        print_metrics=False
    )

    # Run all sequences
    runner.run_sequences(sequence_names=SEQUENCE_NAMES)

    # Calculate RMSE for each sequence and prepare single-column table
    metric_calculator = VINSMetricsCalculator()
    rmse_results = []
    table_data = []

    for sequence_name in SEQUENCE_NAMES:
        gt_file = os.path.join(
            "../src/VINS-Mono/benchmark_publisher/config",
            sequence_name,
            "data.csv"
        )
        est_file = os.path.join(
            OUTPUT_ROOT_FOLDER,
            sequence_name,
            "vins_output",
            "vins_result_no_loop.csv"
        )
        
        # Calculate RMSE
        if os.path.exists(gt_file) and os.path.exists(est_file):
            rmse = metric_calculator.calculate_metric(gt_file, est_file)
            # Round to two decimals and multiply by 100 to convert from meters to cm,
            # in order to get the values in the format reported in the paper.
            rmse = round(rmse, 2) * 100
            rmse_results.append(rmse)
            table_data.append([sequence_name, "{rmse}".format(rmse = rmse)])

    # Add mean and std rows if we have results
    if rmse_results:
        table_data.append(["Mean", "{mean:.1f}".format(mean = np.mean(rmse_results))])
        table_data.append(["Std Dev", "{stdev:.1f}".format(stdev = np.std(rmse_results))])

    # Print the single-column table
    print("\nVINS-Mono RMSE Results:")
    print(tabulate(table_data, headers=["Sequence (cm)", "SEAL + VINS-Mono"], tablefmt="grid"))
