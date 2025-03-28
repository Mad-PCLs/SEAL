import copy
import evo.main_traj
import evo.tools
import numpy as np
import pandas as pd

import evo
from evo.core import metrics, sync
from evo.core.trajectory import PoseTrajectory3D, align_trajectory

class VINSMetricsCalculator:
    """
    A class to calculate RMSE between ground truth and VINS-Mono estimated trajectories.
    Processes VINS-Mono CSV files directly and computes metrics using evo.
    """

    def __init__(self):
        pass

    def _load_vins_csv(self, csv_path: str) -> pd.DataFrame:
        """
        Loads a VINS-Mono CSV file and converts it to TUM format DataFrame.
        
        Args:
            csv_path: Path to VINS-Mono output CSV file
            
        Returns:
            DataFrame with columns: timestamp, tx, ty, tz, qx, qy, qz, qw
        """
        df = pd.read_csv(csv_path, header=None)
        
        # Handle different VINS-Mono output formats
        if len(df.columns) == 12:  # With loop closure (has extra velocity columns)
            df = df.iloc[:, :-3]   # Remove velocity columns
        df = df.iloc[:, :-1]       # Remove empty column from trailing comma
        
        # Rename columns to TUM format
        df.columns = ['timestamp', 'tx', 'ty', 'tz', 'qx', 'qy', 'qz', 'qw']
        
        # Convert timestamp to seconds (but don't make relative)
        df['timestamp'] = df['timestamp'] / 1e9
        
        return df

    def _load_ground_truth(self, ground_truth_path: str) -> pd.DataFrame:
        """
        Loads ground truth CSV file and converts to TUM format DataFrame.
        
        Args:
            ground_truth_path: Path to ground truth CSV file
            
        Returns:
            DataFrame with columns: timestamp, tx, ty, tz, qx, qy, qz, qw
        """
        gt = pd.read_csv(ground_truth_path, header=0)
        gt = gt.astype({"#timestamp": np.float64})
        gt.iloc[:, 0] = gt.iloc[:, 0] / 1e9  # Convert to seconds
        
        # Extract first 8 columns and rename
        gt = gt.iloc[:, :8]
        gt.columns = ["timestamp", "tx", "ty", "tz", "qw", "qx", "qy", "qz"]
        
        # Reorder quaternion to match TUM format (qx, qy, qz, qw)
        gt = gt[["timestamp", "tx", "ty", "tz", "qx", "qy", "qz", "qw"]]
        
        return gt

    def _dataframe_to_trajectory(self, df: pd.DataFrame) -> PoseTrajectory3D:
        """
        Converts DataFrame to evo PoseTrajectory3D object.
        
        Args:
            df: DataFrame with TUM format columns
            
        Returns:
            PoseTrajectory3D object
        """
        return PoseTrajectory3D(
            positions_xyz=df[["tx", "ty", "tz"]].to_numpy(),
            orientations_quat_wxyz=df[["qx", "qy", "qz", "qw"]].to_numpy(),
            timestamps=df["timestamp"].to_numpy()
        )

    def _align_and_compute_rmse(
        self,
        traj_gt: PoseTrajectory3D,
        traj_pred: PoseTrajectory3D,
        max_diff: float = 0.01
    ) -> float:
        """
        Aligns trajectories and computes RMSE.
        
        Args:
            traj_gt: Ground truth trajectory
            traj_pred: Estimated trajectory
            max_diff: Max time difference for association
            
        Returns:
            RMSE after alignment
        """
        traj_gt, traj_pred = sync.associate_trajectories(traj_gt, traj_pred, max_diff)
        traj_pred_aligned = copy.deepcopy(traj_pred)
        traj_pred_aligned = align_trajectory(traj_pred_aligned, traj_gt)
        
        ape_metric = metrics.APE(metrics.PoseRelation.translation_part)
        ape_metric.process_data((traj_gt, traj_pred_aligned))
        
        return ape_metric.get_statistic(metrics.StatisticsType.rmse)

    def calculate_metric(self, gt_file: str, est_file: str) -> float:
        """
        Calculates RMSE between ground truth and VINS-Mono estimated trajectory.
        
        Args:
            gt_file: Path to ground truth CSV
            est_file: Path to VINS-Mono output CSV
            
        Returns:
            RMSE in meters
        """
        df_est = self._load_vins_csv(est_file)
        df_gt = self._load_ground_truth(gt_file)
        
        traj_est = self._dataframe_to_trajectory(df_est)
        traj_gt = self._dataframe_to_trajectory(df_gt)
        return self._align_and_compute_rmse(traj_gt, traj_est)
