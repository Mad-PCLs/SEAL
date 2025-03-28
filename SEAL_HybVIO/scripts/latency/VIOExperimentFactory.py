from typing import Dict
import re

from .VIOExperiment import VIOExperiment
from .VIOFrontendMeasurements import VIOFrontendMeasurements
from .VIOBackendMeasurement import VIOBackendMeasurements

class VIOExperimentFactory:
    @staticmethod
    def parse_output_times(stdout: str) -> Dict[str, float]:
        """
        Parses the raw output file and extracts timing measurements for the VIO frontend and backend.
        """
        # Regex to match function timings
        function_timing_pattern = re.compile(
            r"^\s+([\w\s]+)\s+([\d.]+)$",
            flags=re.MULTILINE,
        )

        # Extract the Odometry section
        odometry_section = re.search(
            r"Odometry Per-frame average timings \[ms\]:",
            stdout,
        )
        if not odometry_section:
            raise ValueError("Odometry section not found in the input string.")

        # Extract the relevant lines
        section_data = {}
        sec = stdout[odometry_section.end() :]
        sec = re.sub(r"\r\n|\r|\n", "\n", sec, flags=re.MULTILINE)
        lines = sec.split("\n")
        for line in lines:
            match = function_timing_pattern.match(line)
            if match:
                function_name = match.group(1).strip().replace(" ", "")
                timing = float(match.group(2))
                section_data[function_name] = timing
            if "FRAMES" in line:
                break

        return section_data

    @staticmethod
    def create_from_file(file_path: str) -> VIOExperiment:
        """
        Creates a VIOExperiment instance by parsing the raw file.
        """
        with open(file_path, "r") as file:
            stdout = file.read()

        parsed_data = VIOExperimentFactory.parse_output_times(stdout)

        frontend = VIOFrontendMeasurements(
            findKeypoints=parsed_data.get("findKeypoints", 0.0),
            computeOpticalFlow=parsed_data.get("computeOpticalFlow", 0.0),
        )

        backend = VIOBackendMeasurements(
            doRansac2=parsed_data.get("doRansac2", 0.0),
            doRansac5=parsed_data.get("doRansac5", 0.0),
            trackerVisualUpdate=parsed_data.get("trackerVisualUpdate", 0.0),
            KFpredict=parsed_data.get("KFpredict", 0.0),
        )

        return VIOExperiment(frontend=frontend, backend=backend)
