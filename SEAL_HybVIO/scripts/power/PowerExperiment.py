import os
import shutil
import subprocess
import re
import time

from typing import Literal

PowerEmulatorMode = Literal["store", "load"]

class PowerExperiment:
    def __init__(
        self,
        id: int,
        executable: str,
        dataset_root_folder: str,
        sequence_name: str,
        orb_vocabulary_path: str,
        seal_power_emulator_storage_path: str,
        seal_power_emulator_mode: PowerEmulatorMode
    ):
        # Identifier variables - configuration specific
        self.id = id
        self.sequence_name = sequence_name
        # Subprocess arguments
        self.executable = executable
        self.dataset_folder = os.path.join(dataset_root_folder, sequence_name)
        self.orb_vocabulary_path = orb_vocabulary_path
        self.seal_power_emulator_storage_path = seal_power_emulator_storage_path
        self.seal_power_emulator_mode: PowerEmulatorMode = seal_power_emulator_mode

        # HybVIO expects the root folder containing all the serialized
        # flow for all sequences as the argument, hence the specific folder
        # for the current sequence is defined below and used in case of an
        # exception or interrupt to delete only that specific folder.
        self.sequence_storage_path = os.path.join(
            seal_power_emulator_storage_path,
            sequence_name
        )

    def is_optFlow_cereal_available(self) -> bool:
        if not os.path.exists(self.sequence_storage_path):
            return False
        if os.listdir(self.sequence_storage_path):
            return True
        return False

    def _start_proc(self, store_opt_flow: bool = False) -> subprocess.Popen:
        args = [
            self.executable,
            f"-vocabularyPath={self.orb_vocabulary_path}",
            f"-i={self.dataset_folder}",
            f"-displayVideo=false",
            f"-timer=true",
            f"-sealPowerEmulatorStoragePath={self.seal_power_emulator_storage_path}"
        ]
        if store_opt_flow:
            args.append("-sealPowerEmulatorMode=store")
        else:
            args.append("-sealPowerEmulatorMode=load")
            if not (store_opt_flow or self.is_optFlow_cereal_available()):
                raise Exception(
                    f"Serialized Optical Flow output for map {self.sequence_name},"
                    f" not found in {self.seal_power_emulator_storage_path}"
                )

        proc = subprocess.Popen(
            args=args,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT
        )
        return proc

    def _parse_output_times(self, stdout: str) -> dict:
        """This will allow us to make sure that the execution was successful"""
        # Define the patterns for section headers and function timings
        section_headers = {
            "Odometry": r"Odometry Per-frame average timings \[ms\]:",
            "Main": r"Main Per-frame average timings \[ms\]:"
        }

        # Regex to match function timings
        function_timing_pattern = re.compile(
            r"^\s+([\w\s]+)\s+([\d.]+)$",
            flags=re.MULTILINE
        )

        # Extract the sections and corresponding timings
        section_timings = {
            "id": self.id,
            "dataset_id": self.sequence_name,
        }
        for section, header_pattern in section_headers.items():
            section_start = re.search(header_pattern, stdout)
            if not section_start:
                continue
            section_data = {}
            sec = stdout[section_start.end():]
            sec = re.sub(r'\r\n|\r|\n', '\n', sec, flags=re.MULTILINE)
            lines = sec.split('\n')
            for line in lines:
                match = function_timing_pattern.match(line)
                if match:
                    function_name = match.group(1).strip().replace(' ', '')
                    timing = float(match.group(2))
                    suffix = "_ms" if not 'FRAMES' in line else ''
                    section_data[f"{section.lower()}_{function_name}{suffix}"] = timing
                # Break when we hit the end of the section
                if 'FRAMES' in line:
                    break
            # Append extra columns to the experiment row
            section_timings.update(section_data)

        if len(section_timings) == 4:
            raise ValueError("No valid sections or function timings found in the input string.")
        return section_timings

    def _is_output_valid(self, stdout: str) -> bool:
        out_dict = self._parse_output_times(stdout)
        # Check if the timing values return are at least acceptable
        # TODO: This is not the best way to verify if a run is valid. Fix it.
        keys = ["odometry_TOTAL_ms", "odometry_FRAMES", "main_TOTAL_ms", "main_FRAMES"]
        min_vals = [5., 1000, 5., 1000]
        for key, min_val in zip(keys, min_vals):
            recorder_res = float(out_dict[key])
            if recorder_res < float(min_val):
                raise Exception(f"Timing measurement {recorder_res} for {key} is too low!")
        return True

    def optFlow_available_guard(self):
        # If we are not to load the optical flow,
        # we don't really need to check if it has been pre-computed
        if self.seal_power_emulator_mode != "load":
            return

        # If we are going to load the optical flow and it has
        # already been pre-computed and stored in self.seal_power_emulator_storage_path
        # the guard has nothing else to do.
        if self.is_optFlow_cereal_available():
            return

        # If we are going to load and we didn't find the pre-computed
        # flow, calculate it and store it in self.seal_power_emulator_storage_path
        proc = self._start_proc(store_opt_flow=True)
        proc.wait()
        out, _ = proc.communicate()
        out: bytes
        if not (self._is_output_valid(out.decode('utf-8')) or self.is_optFlow_cereal_available()):
            raise Exception("Error occured while attempting to store optical flow results!")

    def run_me(self) -> tuple[int, float]:
        """Returns the pid and the execution time in seconds."""
        try:
            t_start = time.time()
            proc = self._start_proc(
                store_opt_flow=(self.seal_power_emulator_mode == "store")
            )
            proc.wait()
            time_delta = time.time() - t_start

            out, _ = proc.communicate()
            out: bytes
            self._is_output_valid(out.decode('utf-8'))

            return proc.pid, time_delta

        except (KeyboardInterrupt, Exception) as e:
            if os.path.exists(self.sequence_storage_path):
                print(f"Error occurred: {e}. Deleting sequence folder: {self.sequence_storage_path}")
                shutil.rmtree(self.sequence_storage_path)
            raise
