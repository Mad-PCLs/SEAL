import argparse
import os
import numpy as np
from tabulate import tabulate
from accuracy.SequenceRunner import SequenceRunner
from accuracy.OriginalHybVIOMetricsCalculator import OriginalHybVIOMetricsCalculator

BENCHMARK_FOLDER = "../data/benchmark/"
RAW_DATA_FOLDER = "../data/raw/"
OUTPUT_ROOT_FOLDER = "../output_accuracy/table10/"
BINARY_PATH = "../HybVIO/build/main"
ORB_VOCAB_PATH = "../HybVIO/data/orb_vocab.dbow2"
SEAL_PARAMS_PATH = "../HybVIO/SEAL/seal_params.yaml"

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

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Run HybVIO on sequences and calculate metrics."
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

    runner = SequenceRunner(
        benchmarks_folder=BENCHMARK_FOLDER,
        raw_data_folder=RAW_DATA_FOLDER,
        output_root_folder=OUTPUT_ROOT_FOLDER,
        binary_path=BINARY_PATH,
        orb_vocab_path=ORB_VOCAB_PATH,
        seal_params_path=SEAL_PARAMS_PATH,
        metric="RMSE",
        print_metrics=False
    )

    runner.run_sequences(sequence_names=SEQUENCE_NAMES, rerun=args.rerun)

    # Calculate RMSE for each sequence and prepare single-column table
    metric_calculator = OriginalHybVIOMetricsCalculator(metric="RMSE")
    rmse_results = []
    table_data = []

    for sequence_name in SEQUENCE_NAMES:
        gt_file = os.path.join(RAW_DATA_FOLDER, sequence_name, "mav0", "state_groundtruth_estimate0", "data.csv")
        est_file = os.path.join(OUTPUT_ROOT_FOLDER, sequence_name, "estimated_trajectory.jsonl")
        
        # Calculate RMSE
        rmse = metric_calculator.calculate_metric(gt_file, est_file)
        # Round to two decimals and multiply by 100 to convert from meters to cm,
        # in order to get the values in the format reported in the paper.
        rmse = round(rmse, 2) * 100
        rmse_results.append(rmse)
        table_data.append([sequence_name, f"{rmse}"])

    # Add mean and std rows
    if len(rmse_results) > 1:  # Only if we have multiple sequences
        table_data.append(["Mean", f"{np.mean(rmse_results):.1f}"])
        table_data.append(["Std Dev", f"{np.std(rmse_results):.1f}"])

    # Print the single-column table
    print("\nSEAL + HybVIO RMSE Results:")
    print(tabulate(table_data, headers=["Sequence (cm)", "SEAL + HybVIO"], tablefmt="grid"))
