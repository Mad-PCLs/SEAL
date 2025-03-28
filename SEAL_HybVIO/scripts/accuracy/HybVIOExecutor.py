import os
import subprocess
import shutil
import json
from datetime import datetime

class HybVIOExecutor:
    def __init__(
        self,
        benchmarks_folder: str,
        sequence_name: str,
        raw_data_folder: str,
        output_root_folder: str,
        binary_path: str,
        orb_vocab_path: str,
        seal_params_path: str
    ):
        """
        Initialize the HybVIOExecutor.

        Args:
            benchmarks_folder (str): Path to the folder containing all benchmarks.
            sequence_name (str): Name of the sequence (e.g., "euroc-mh-05-difficulty").
            raw_data_folder (str): Path to the folder containing raw data (including ground truth).
            output_root_folder (str): Path to the root folder where output for each sequence will be stored.
            binary_path (str): Path to the HybVIO binary.
            orb_vocab_path (str): Path to the ORB vocabulary file (e.g., "orb_vocab.dbow2").
            seal_params_path (str): Path to the SEAL parameters file.
        """
        self.benchmarks_folder = benchmarks_folder
        self.sequence_name = sequence_name
        self.raw_data_folder = raw_data_folder
        self.output_root_folder = output_root_folder
        self.binary_path = binary_path
        self.orb_vocab_path = orb_vocab_path
        self.seal_params_path = seal_params_path

        # Derived paths
        self.benchmark_folder_for_sequence = os.path.join(self.benchmarks_folder, self.sequence_name)
        self.raw_data_folder_for_sequence = os.path.join(self.raw_data_folder, self.sequence_name)
        self.output_folder_for_sequence = os.path.join(self.output_root_folder, self.sequence_name)
        self.ground_truth_path = os.path.join(
            self.raw_data_folder_for_sequence, "mav0", "state_groundtruth_estimate0", "data.csv"
        )

    def _create_output_folder(self) -> bool:
        """
        Create the output folder for the sequence if it doesn't exist.

        Returns:
            bool: True if the folder was created, False if it already exists and is not empty.
        """
        if os.path.exists(self.output_folder_for_sequence) and os.listdir(self.output_folder_for_sequence):
            print(f"Output folder for sequence '{self.sequence_name}' already exists and is not empty. Skipping execution.")
            return False
        os.makedirs(self.output_folder_for_sequence, exist_ok=True)
        return True

    def _copy_seal_params(self):
        """Copy the SEAL parameters file to the output folder."""
        shutil.copy(self.seal_params_path, self.output_folder_for_sequence)

    def _run_hybvio(self):
        """
        Run the HybVIO binary as a subprocess.

        Raises:
            subprocess.CalledProcessError: If the subprocess exits with a non-zero return code.
        """
        estimated_trajectory_path = os.path.join(self.output_folder_for_sequence, "estimated_trajectory.jsonl")
        stdout_path = os.path.join(self.output_folder_for_sequence, "stdout.txt")

        command = [
            self.binary_path,
            f"-vocabularyPath={self.orb_vocab_path}",
            f"-sealParamsFile={self.seal_params_path}",
            "-maxSuccessfulVisualUpdates=20",
            f"-i={self.benchmark_folder_for_sequence}",
            f"-o={estimated_trajectory_path}",
            "-timer=true",
            "-displayVideo=false"
        ]

        with open(stdout_path, "w") as stdout_file:
            result = subprocess.run(
                command,
                stdout=stdout_file,
                stderr=subprocess.STDOUT,
                text=True,
            )

        if result.returncode != 0:
            raise subprocess.CalledProcessError(result.returncode, command)

    def _log_execution_info(self):
        """Log execution information to a JSON file in the output folder."""
        execution_info = {
            "sequence_name": self.sequence_name,
            "execution_time": datetime.now().isoformat(),
            "binary_path": self.binary_path,
            "orb_vocab_path": self.orb_vocab_path,
            "seal_params_path": self.seal_params_path,
            "benchmark_folder": self.benchmark_folder_for_sequence,
            "raw_data_folder": self.raw_data_folder_for_sequence,
            "output_folder": self.output_folder_for_sequence,
        }

        execution_info_path = os.path.join(self.output_folder_for_sequence, "execution_info.json")
        with open(execution_info_path, "w") as f:
            json.dump(execution_info, f, indent=4)

    def _handle_error(self):
        """Rename the output folder to indicate an invalid execution."""
        invalid_folder_name = f"invalid_{self.sequence_name}"
        invalid_folder_path = os.path.join(self.output_root_folder, invalid_folder_name)
        if os.path.exists(invalid_folder_path):
            shutil.rmtree(invalid_folder_path)
        os.rename(self.output_folder_for_sequence, invalid_folder_path)
        print(f"Execution failed. Renamed output folder to '{invalid_folder_name}'.")

    def _cleanup_on_interrupt(self):
        """Clean up the output folder on keyboard interrupt."""
        print("Keyboard interrupt detected. Cleaning up...")
        if os.path.exists(self.output_folder_for_sequence):
            shutil.rmtree(self.output_folder_for_sequence)
        print("Cleanup complete. Exiting.")

    def execute(self):
        """
        Execute the HybVIO binary for the specified sequence.

        Returns:
            Optional[str]: Path to the estimated trajectory file if successful, None otherwise.
        """
        try:
            # Check if the output folder already exists and is not empty
            if not self._create_output_folder():
                return None

            self._copy_seal_params()
            self._run_hybvio()
            self._log_execution_info()

            # Print paths for debugging
            estimated_trajectory_path = os.path.join(self.output_folder_for_sequence, "estimated_trajectory.jsonl")
            print(f"Estimated trajectory: {estimated_trajectory_path}")
            print(f"Ground truth: {self.ground_truth_path}")

            return estimated_trajectory_path

        except subprocess.CalledProcessError as e:
            print(f"HybVIO execution failed with return code {e.returncode}.")
            self._handle_error()
            raise

        except KeyboardInterrupt:
            self._cleanup_on_interrupt()
            raise
