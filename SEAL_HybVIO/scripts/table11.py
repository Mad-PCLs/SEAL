import os
import numpy as np
from tabulate import tabulate
from accuracy.SequenceRunner import SequenceRunner
from accuracy.OriginalHybVIOMetricsCalculator import OriginalHybVIOMetricsCalculator

# Global variables
BENCHMARK_FOLDER = "../data/benchmark/"
RAW_DATA_FOLDER = "../data/raw/"
OUTPUT_ROOT_FOLDER = "../output_accuracy/table11/"
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

EDGE_OUTPUT_FOLDER_NAMES = [
    "17",
    "18",
    "21",
    "22",
    "24",
    "30"
]

SEAL_PARAMS_FILES = [
    "../HybVIO/SEAL/table11_edges/seal_params_edge_17.yaml",
    "../HybVIO/SEAL/table11_edges/seal_params_edge_18.yaml",
    "../HybVIO/SEAL/table11_edges/seal_params_edge_21.yaml",
    "../HybVIO/SEAL/table11_edges/seal_params_edge_22.yaml",
    "../HybVIO/SEAL/table11_edges/seal_params_edge_24.yaml",
    "../HybVIO/SEAL/table11_edges/seal_params_edge_30.yaml"
]

if __name__ == "__main__":
    # Dictionary to store all RMSE results
    all_rmse_results = {}
    metric_calculator = OriginalHybVIOMetricsCalculator(metric="RMSE")

    # First loop: Process each configuration (folder)
    for folder_name, seal_params_file in zip(EDGE_OUTPUT_FOLDER_NAMES, SEAL_PARAMS_FILES):
        print("")
        print("*"*10 + f" Running for edge threshold: {folder_name} " + "*"*10)
        curr_output_root_folder = os.path.join(OUTPUT_ROOT_FOLDER, folder_name)

        # Create the folder if it doesn't exist
        os.makedirs(curr_output_root_folder, exist_ok=True)

        # Run SequenceRunner for this configuration
        runner = SequenceRunner(
            benchmarks_folder=BENCHMARK_FOLDER,
            raw_data_folder=RAW_DATA_FOLDER,
            output_root_folder=curr_output_root_folder,
            binary_path=BINARY_PATH,
            orb_vocab_path=ORB_VOCAB_PATH,
            seal_params_path=seal_params_file,
            metric="RMSE",
            print_metrics=False
        )

        # Run for specific sequence or all sequences
        runner.run_sequences(sequence_names=SEQUENCE_NAMES)

        # Calculate RMSE for each sequence in this configuration
        rmse_results = []
        for sequence_name in SEQUENCE_NAMES:
            gt_file = os.path.join(RAW_DATA_FOLDER, sequence_name, "mav0", "state_groundtruth_estimate0", "data.csv")
            est_file = os.path.join(curr_output_root_folder, sequence_name, "estimated_trajectory.jsonl")

            # Calculate RMSE
            rmse = metric_calculator.calculate_metric(gt_file, est_file)
            # Round to two decimals and multiply by 100 to convert from meters to cm,
            # in order to get the values in the format reported in the paper.
            rmse = round(rmse, 2)*100
            rmse_results.append(rmse)

        # Store results for this configuration
        all_rmse_results[folder_name] = rmse_results

    # Prepare and print the results table
    # Prepare table data
    table_data = []
    best_values = []  # To store best (minimum) values for each sequence
    
    for i, sequence_name in enumerate(SEQUENCE_NAMES):
        row = [sequence_name]
        sequence_values = []
        for folder_name in EDGE_OUTPUT_FOLDER_NAMES:
            value = all_rmse_results[folder_name][i]
            row.append(f"{value}")
            sequence_values.append(value)
        
        # Find the best (minimum) value for this sequence
        best_value = min(sequence_values)
        best_values.append(best_value)
        row.append(f"{best_value}")  # Add best value column
        table_data.append(row)
    
    # Calculate mean and std for each threshold and for best values
    means = ["Mean"]
    stds = ["Std Dev"]
    
    # Calculate for each threshold
    for folder_name in EDGE_OUTPUT_FOLDER_NAMES:
        values = all_rmse_results[folder_name]
        means.append(f"{np.mean(values):.1f}")
        stds.append(f"{np.std(values):.1f}")
    
    # Calculate for best values column
    means.append(f"{np.mean(best_values):.1f}")
    stds.append(f"{np.std(best_values):.1f}")
    
    # Add mean and std rows to the table
    table_data.append(means)
    table_data.append(stds)
        
    # Print the table
    headers = ["Sequence (cm)"] + EDGE_OUTPUT_FOLDER_NAMES + ["Best"]
    print("\nRMSE Results:")
    print(tabulate(table_data, headers=headers, tablefmt="grid"))
