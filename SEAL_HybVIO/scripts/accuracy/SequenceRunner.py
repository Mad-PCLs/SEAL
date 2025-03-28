import os
from typing import Optional, List, Literal
import yaml
import shutil

from .HybVIOExecutor import HybVIOExecutor
from .OriginalHybVIOMetricsCalculator import OriginalHybVIOMetricsCalculator
from .EVOMetricsCalculator import EVOMetricsCalculator

class SequenceRunner:
    def __init__(
        self,
        benchmarks_folder: str,
        raw_data_folder: str,
        output_root_folder: str,
        binary_path: str,
        orb_vocab_path: str,
        seal_params_path: str,
        metric: Literal["MAE", "RMSE", "average", "evoRMSE"] = "RMSE",
        print_metrics: bool = True
    ):
        """
        Initialize the SequenceRunner.

        Args:
            benchmarks_folder (str): Path to the folder containing all benchmarks.
            raw_data_folder (str): Path to the folder containing raw data (including ground truth).
            output_root_folder (str): Path to the root folder where output for each sequence will be stored.
            binary_path (str): Path to the HybVIO binary.
            orb_vocab_path (str): Path to the ORB vocabulary file (e.g., "orb_vocab.dbow2").
            seal_params_path (str): Path to the SEAL parameters file.
            print_metrics (bool): Print the RMSE at the end of each sequence and the mean after all sequences complete.
        """
        self.benchmarks_folder = benchmarks_folder
        self.raw_data_folder = raw_data_folder
        self.output_root_folder = output_root_folder
        self.binary_path = binary_path
        self.orb_vocab_path = orb_vocab_path
        self.seal_params_path = seal_params_path
        self.metric = metric
        self.print_metrics = print_metrics

        if metric == "evoRMSE":
            self.metric_calculator = EVOMetricsCalculator()
        else:
            self.metric_calculator = OriginalHybVIOMetricsCalculator(
                metric=metric
            )

    def _check_seal_params_difference(self, sequence_name: str) -> bool:
        """
        Check if the SEAL parameters file in the output folder differs from the current one.

        Args:
            sequence_name (str): Name of the sequence.

        Returns:
            bool: True if the files differ, False otherwise.
        """
        output_folder = os.path.join(self.output_root_folder, sequence_name)
        existing_seal_params_path = os.path.join(output_folder, os.path.basename(self.seal_params_path))

        if not os.path.exists(existing_seal_params_path):
            return False

        with open(self.seal_params_path, "r") as f1, open(existing_seal_params_path, "r") as f2:
            lines1 = f1.readlines()
            lines2 = f2.readlines()
            if lines1[0].startswith("%YAML"): lines1 = lines1[1:]
            if lines2[0].startswith("%YAML"): lines2 = lines2[1:]

            current_params = yaml.safe_load("".join(lines1))
            existing_params = yaml.safe_load("".join(lines2))

        return current_params != existing_params

    def _delete_output_folder(self, sequence_name: str):
        """
        Delete the output folder for the sequence if it exists.

        Args:
            sequence_name (str): Name of the sequence.
        """
        output_folder = os.path.join(self.output_root_folder, sequence_name)
        if os.path.exists(output_folder):
            shutil.rmtree(output_folder)
            print(f"Deleted output folder for sequence '{sequence_name}'.")

    def run_sequences(
        self,
        sequence_names: Optional[List[str]] = None,
        rerun: bool = False,
    ) -> float:
        """
        Run all sequences (or a specific sequence) and calculate the average metric.

        Args:
            sequence_names (Optional[List[str]]): List of sequence names to run. If None, run all sequences.
            rerun (bool): Whether to delete existing output folders and rerun the sequences.

        Returns:
            float: The average metric across all sequences.
        """
        if sequence_names is None:
            # Get all sequence names from the benchmarks folder
            sequence_names = [
                name for name in os.listdir(self.benchmarks_folder)
                if os.path.isdir(os.path.join(self.benchmarks_folder, name))
            ]

        total_metric = 0.0
        num_sequences = 0

        for sequence_name in sequence_names:
            print(f"\nProcessing sequence: {sequence_name}")

            # Delete output folder if --rerun is specified
            if rerun:
                self._delete_output_folder(sequence_name)

            # Check if the output folder already exists and is not empty
            output_folder = os.path.join(self.output_root_folder, sequence_name)
            if os.path.exists(output_folder) and os.listdir(output_folder):
                # Check if the SEAL parameters file differs
                if self._check_seal_params_difference(sequence_name):
                    print(
                        f"WARNING: SEAL parameters file for sequence '{sequence_name}' differs from the current one. "
                        "Consider rerunning with --rerun."
                    )

                # Calculate the metric for the existing output
                filt_seq_name = sequence_name.removesuffix("-cam0").removesuffix("-cam1")
                gt_file = os.path.join(
                    self.raw_data_folder, filt_seq_name, "mav0", "state_groundtruth_estimate0", "data.csv"
                )
                est_file = os.path.join(output_folder, "estimated_trajectory.jsonl")

                metric_value = self.metric_calculator.calculate_metric(gt_file, est_file)
                if self.print_metrics:
                    print(f"Metric for sequence '{sequence_name}': {metric_value:.2f}")

                total_metric += metric_value
                num_sequences += 1
                continue

            # Run HybVIOExecutor for the sequence
            executor = HybVIOExecutor(
                benchmarks_folder=self.benchmarks_folder,
                sequence_name=sequence_name,
                raw_data_folder=self.raw_data_folder,
                output_root_folder=self.output_root_folder,
                binary_path=self.binary_path,
                orb_vocab_path=self.orb_vocab_path,
                seal_params_path=self.seal_params_path,
            )
            estimated_trajectory_path = executor.execute()

            if estimated_trajectory_path is None:
                print(f"Skipping sequence '{sequence_name}' due to execution failure.")
                continue

            # Calculate the metric for the new output
            gt_file = os.path.join(
                self.raw_data_folder, sequence_name, "mav0", "state_groundtruth_estimate0", "data.csv"
            )
            metric_value = self.metric_calculator.calculate_metric(gt_file, estimated_trajectory_path)
            if self.print_metrics:
                print(f"Metric for sequence '{sequence_name}': {metric_value:.2f}")

            total_metric += metric_value
            num_sequences += 1

        if num_sequences == 0:
            print("No sequences processed.")
            return 0.0

        # Calculate the overall average metric
        overall_average = total_metric / num_sequences
        if self.print_metrics:
            print(f"\nOverall average metric across {num_sequences} sequences: {overall_average:.3f}")
        return overall_average
