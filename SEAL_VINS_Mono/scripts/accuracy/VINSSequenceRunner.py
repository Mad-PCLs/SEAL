import os
import shutil
from typing import Optional, List
import yaml

from accuracy.VINSMonoExecutor import VINSMonoExecutor
from accuracy.VINSMetricsCalculator import VINSMetricsCalculator

class VINSSequenceRunner:
    def __init__(
        self,
        data_folder: str,
        output_root_folder: str,
        seal_params_path: str,
        vins_package: str,
        vins_launch_file: str,
        default_output_folder: str,
        ground_truth_root_folder: str,
        setup_script_path: str,
        print_metrics: bool = True
    ):
        """
        Initialize the VINS-Mono SequenceRunner.

        Args:
            data_folder: Path to the folder containing rosbag data
            output_root_folder: Root folder where output for each sequence will be stored
            seal_params_path: Path to the SEAL parameters file
            vins_package: Name of the VINS ROS package
            vins_launch_file: Name of the VINS launch file
            default_output_folder: Default output folder created by VINS
            ground_truth_root_folder: Root folder containing ground truth data
            setup_script_path: Path to the setup.bash script to source
            print_metrics: Print the RMSE at the end of each sequence and the mean after all sequences complete.
        """
        self.data_folder = data_folder
        self.output_root_folder = output_root_folder
        self.seal_params_path = seal_params_path
        self.vins_package = vins_package
        self.vins_launch_file = vins_launch_file
        self.default_output_folder = default_output_folder
        self.ground_truth_root_folder = ground_truth_root_folder
        self.setup_script_path = setup_script_path
        self.print_metrics = print_metrics
        self.metric_calculator = VINSMetricsCalculator()

    def _check_seal_params_difference(self, sequence_name: str) -> bool:
        """
        Check if the SEAL parameters file in the output folder differs from the current one.
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
        """Delete the output folder for the sequence if it exists."""
        output_folder = os.path.join(self.output_root_folder, sequence_name)
        if os.path.exists(output_folder):
            shutil.rmtree(output_folder)
            print("Deleted output folder for sequence '{}'.".format(sequence_name))

    def run_sequences(
        self,
        sequence_names: Optional[List[str]] = None,
        rerun: bool = False,
    ) -> float:
        """
        Run sequences and calculate average RMSE.

        Args:
            sequence_names: List of sequence names to run (None for all)
            rerun: Whether to delete existing output folders and rerun

        Returns:
            Average RMSE across all sequences
        """
        if sequence_names is None:
            sequence_names = [
                os.path.splitext(f)[0] for f in os.listdir(self.data_folder)
                if f.endswith('.bag')
            ]

        total_rmse = 0.0
        num_sequences = 0

        for sequence_name in sequence_names:
            print("\nProcessing sequence: {}".format(sequence_name))

            if rerun:
                self._delete_output_folder(sequence_name)

            output_folder = os.path.join(self.output_root_folder, sequence_name)
            if os.path.exists(output_folder) and os.listdir(output_folder):
                if self._check_seal_params_difference(sequence_name):
                    print("WARNING: SEAL parameters file for sequence '{}' differs. "
                          "Consider rerunning with --rerun.".format(sequence_name))

                gt_file = os.path.join(
                    self.ground_truth_root_folder, 
                    sequence_name, 
                    "data.csv"
                )
                est_file = os.path.join(output_folder, "vins_output", "vins_result_no_loop.csv")

                rmse = self.metric_calculator.calculate_metric(gt_file, est_file)
                if self.print_metrics:
                    print("RMSE for sequence '{}': {:.2f}".format(sequence_name, rmse))

                total_rmse += rmse
                num_sequences += 1
                continue

            executor = VINSMonoExecutor(
                output_root_folder=output_folder,
                rosbag_path=os.path.join(self.data_folder, sequence_name + ".bag"),
                vins_launch_file=self.vins_launch_file,
                seal_params_path=self.seal_params_path,
                vins_package=self.vins_package,
                default_output_folder=self.default_output_folder,
                setup_script_path=self.setup_script_path
            )
            
            try:
                executor.execute()
            except Exception as e:
                print("Error running sequence '{}': {}".format(sequence_name, str(e)))
                continue

            gt_file = os.path.join(
                self.ground_truth_root_folder,
                sequence_name, 
                "data.csv"
            )
            est_file = os.path.join(output_folder, "vins_output", "vins_result_no_loop.csv")
            
            rmse = self.metric_calculator.calculate_metric(gt_file, est_file)
            if self.print_metrics:
                print("RMSE for sequence '{}': {:.2f}".format(sequence_name, rmse))

            total_rmse += rmse
            num_sequences += 1

        if num_sequences == 0:
            print("No sequences processed.")
            return 0.0

        overall_average = total_rmse / num_sequences
        if self.print_metrics:
            print("\nOverall average RMSE across {} sequences: {:.4f}".format(num_sequences, overall_average))
        return overall_average
