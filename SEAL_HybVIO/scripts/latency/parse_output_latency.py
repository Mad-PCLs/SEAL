# Processes latency measurements for each sequence,
# accumulates results, and calculates averages.
# Prints the average values for each sequence and
# the overall average across all sequences.

import os
import sys

from latency.VIOExperimentFactory import VIOExperimentFactory
from latency.VIOFrontendMeasurements import VIOFrontendMeasurements
from latency.VIOBackendMeasurement import VIOBackendMeasurements

ROOT_PATH = "../output_latency"

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

# Example usage
if __name__ == "__main__":
    # Initialize accumulators for overall averages
    overall_total_experiments = 0
    overall_total_frontend = VIOFrontendMeasurements()
    overall_total_backend = VIOBackendMeasurements()

    for sequence_name in SEQUENCE_NAMES:
        sequence_path = os.path.join(ROOT_PATH, sequence_name, "stdout")
        if not os.path.isdir(sequence_path):
            print(f"Directory not found for sequence: {sequence_name}")
            continue

        # Initialize accumulators for the current sequence
        total_experiments = 0
        total_frontend = VIOFrontendMeasurements()
        total_backend = VIOBackendMeasurements()

        # Iterate over all files in the sequence directory
        for file_name in os.listdir(sequence_path):
            if file_name.endswith(".txt"):
                file_path = os.path.join(sequence_path, file_name)
                try:
                    experiment = VIOExperimentFactory.create_from_file(file_path)
                    total_frontend += experiment.frontend
                    total_backend += experiment.backend
                    total_experiments += 1

                    # Add to overall totals
                    overall_total_frontend += experiment.frontend
                    overall_total_backend += experiment.backend
                    overall_total_experiments += 1
                except Exception as e:
                    print(f"Error processing file {file_name} in sequence {sequence_name}: {e}")

        if total_experiments == 0:
            print(f"No valid experiments found for sequence: {sequence_name}")
            continue

        # Calculate averages for the current sequence
        avg_frontend = total_frontend / total_experiments
        avg_backend = total_backend / total_experiments

        # Map measurements to the paper format
        keypoint_detection = avg_frontend.findKeypoints
        keypoint_tracking = avg_frontend.computeOpticalFlow
        ransac = avg_backend.doRansac2 + avg_backend.doRansac5
        backend = avg_backend.trackerVisualUpdate + avg_backend.KFpredict

        # Print results for the sequence
        print(f"Sequence: {sequence_name}")
        print(f"  Keypoint Detection (KD): {keypoint_detection:.4f} ms")
        print(f"  Keypoint Tracking (KT): {keypoint_tracking:.4f} ms")
        print(f"  RANSAC: {ransac:.4f} ms")
        print(f"  Backend (BE): {backend:.4f} ms")
        print()

    # Calculate overall averages
    if overall_total_experiments == 0:
        print("No valid experiments found across all sequences.")
        sys.exit(0)

    overall_avg_frontend = overall_total_frontend / overall_total_experiments
    overall_avg_backend = overall_total_backend / overall_total_experiments

    # Map overall measurements to the paper format
    overall_keypoint_detection = overall_avg_frontend.findKeypoints
    overall_keypoint_tracking = overall_avg_frontend.computeOpticalFlow
    overall_ransac = overall_avg_backend.doRansac2 + overall_avg_backend.doRansac5
    overall_backend = overall_avg_backend.trackerVisualUpdate + overall_avg_backend.KFpredict

    # Print overall results
    print("Overall Averages Across All Sequences:")
    print(f"  Keypoint Detection (KD): {overall_keypoint_detection:.4f} ms")
    print(f"  Keypoint Tracking (KT): {overall_keypoint_tracking:.4f} ms")
    print(f"  RANSAC: {overall_ransac:.4f} ms")
    print(f"  Backend (BE): {overall_backend:.4f} ms")
