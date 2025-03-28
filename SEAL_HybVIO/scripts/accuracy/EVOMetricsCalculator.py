import copy
import json
import numpy as np
import pandas as pd

from evo.core import metrics, sync
from evo.core.trajectory import PoseTrajectory3D

from utils import Position, Orientation

class EVOMetricsCalculator:
    """
    A class to calculate the RMSE between ground truth and estimated trajectories.
    """

    def __init__(self):
        pass

    def _jsonl_to_tum_dataframe(self, jsonl_path: str) -> pd.DataFrame:
        """
        Converts a JSONL file containing HybVIO predictions to a Pandas DataFrame in TUM format.
        Ignores entries with negative timestamps.

        Args:
            jsonl_path (str): Path to the input JSONL file.

        Returns:
            pd.DataFrame: A DataFrame with columns: timestamp, tx, ty, tz, qx, qy, qz, qw.
        """
        data = []

        with open(jsonl_path, "r") as file:
            for line in file:
                # Parse JSON line
                prediction = json.loads(line.strip())

                # Extract data
                timestamp = prediction["time"]
                if timestamp < 0:  # Ignore entries with negative timestamps
                    continue

                position = Position(**prediction["position"])
                orientation = Orientation(**prediction["orientation"])

                # Append to data list in TUM format
                data.append([
                    timestamp,
                    position.x, position.y, position.z,
                    orientation.x, orientation.y, orientation.z, orientation.w
                ])

        # Create DataFrame
        columns = ["timestamp", "tx", "ty", "tz", "qx", "qy", "qz", "qw"]
        df = pd.DataFrame(data, columns=columns)

        return df

    def _load_ground_truth(self, ground_truth_path: str) -> pd.DataFrame:
        """
        Loads the ground truth CSV file, converts timestamps from nanoseconds to seconds,
        and makes them relative to the first timestamp. Skips the header row.

        Args:
            ground_truth_path (str): Path to the ground truth CSV file.

        Returns:
            pd.DataFrame: A DataFrame with columns: timestamp, tx, ty, tz, qx, qy, qz, qw.
        """
        # Load the ground truth CSV file, skipping the header row
        ground_truth = pd.read_csv(ground_truth_path, header=0, sep=",")
        
        # Convert timestamps from nanoseconds to seconds
        ground_truth = ground_truth.astype({"#timestamp": np.float64})
        ground_truth.iloc[:, 0] = ground_truth.iloc[:, 0] / 1e9
        
        # Make timestamps relative to the first timestamp
        ground_truth.iloc[:, 0] = ground_truth.iloc[:, 0] - ground_truth.iloc[0, 0]

        # Extract the first 8 columns
        ground_truth = ground_truth.iloc[:, :8]
        
        # Rename columns
        ground_truth.columns = ["timestamp", "tx", "ty", "tz", "qw", "qx", "qy", "qz"]
        
        return ground_truth

    def _dataframe_to_trajectory(self, df: pd.DataFrame) -> PoseTrajectory3D:
        """
        Converts a Pandas DataFrame to an evo PoseTrajectory3D object.

        Args:
            df (pd.DataFrame): DataFrame with columns: timestamp, tx, ty, tz, qx, qy, qz, qw.

        Returns:
            PoseTrajectory3D: An evo trajectory object.
        """
        # Extract data from DataFrame
        timestamps = df["timestamp"].to_numpy()
        positions = df[["tx", "ty", "tz"]].to_numpy()
        quaternions = df[["qx", "qy", "qz", "qw"]].to_numpy()
        
        # Create PoseTrajectory3D object
        trajectory = PoseTrajectory3D(
            positions_xyz=positions,
            orientations_quat_wxyz=quaternions,
            timestamps=timestamps
        )
        
        return trajectory

    def _align_and_compute_rmse(
        self,
        traj_gt: PoseTrajectory3D,
        traj_pred: PoseTrajectory3D,
        max_diff: float = 0.01
    ) -> float:
        """
        Aligns the predicted trajectory to the ground truth using full_3d alignment
        and computes the RMSE.

        Args:
            traj_gt (PoseTrajectory3D): Ground truth trajectory.
            traj_pred (PoseTrajectory3D): Predicted trajectory.
            max_diff (float): Maximum allowed timestamp difference for association.

        Returns:
            float: The RMSE after alignment.
        """
        # Associate trajectories based on timestamps
        traj_gt, traj_pred = sync.associate_trajectories(traj_gt, traj_pred, max_diff)

        # Align the predicted trajectory to the ground truth
        traj_pred_aligned = copy.deepcopy(traj_pred)
        traj_pred_aligned.align(traj_gt, correct_scale=False, correct_only_scale=False)

        # Compute Absolute Pose Error (APE)
        pose_relation = metrics.PoseRelation.translation_part
        ape_metric = metrics.APE(pose_relation)
        ape_metric.process_data((traj_gt, traj_pred_aligned))

        # Extract RMSE
        rmse = ape_metric.get_statistic(metrics.StatisticsType.rmse)
        return rmse

    def calculate_metric(self, gt_file: str, est_file: str) -> float:
        """
        Calculates the RMSE between the ground truth and estimated trajectories.

        Args:
            gt_file (str): Path to the ground truth trajectory file (TUM format).
            est_file (str): Path to the estimated trajectory file (TUM format).

        Returns:
            float: The RMSE after alignment.
        """
        # Load and convert trajectories
        df_est = self._jsonl_to_tum_dataframe(est_file)
        df_gt = self._load_ground_truth(gt_file)

        traj_est = self._dataframe_to_trajectory(df_est)
        traj_gt = self._dataframe_to_trajectory(df_gt)

        # Compute RMSE
        rmse = self._align_and_compute_rmse(traj_gt, traj_est)
        return rmse
