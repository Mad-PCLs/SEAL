# The methods of the class below are heavily based to:
# https://github.com/AaltoML/vio_benchmark/blob/main/benchmark/compute_metrics.py
# We simply organized only the relevant code into a class for easier use.

import numpy as np
import json
from typing import Literal

class OriginalHybVIOMetricsCalculator:
    def __init__(
        self,
        metric: Literal["MAE", "RMSE", "average"] = "average"
    ):
        """
        Initialize the OriginalHybVIOMetrics object.

        Args:
            metric (Literal["MAE", "RMSE", "average"]): The metric to calculate. Defaults to "average".
        """
        self.metric: Literal["MAE", "RMSE", "average"] = metric

    def _rmse(self, a: np.ndarray, b: np.ndarray) -> float:
        """Root Mean Square Error"""
        return np.sqrt(np.mean(np.sum((a - b) ** 2, axis=1)))

    def _mean_absolute_error(self, a: np.ndarray, b: np.ndarray) -> float:
        """Mean Absolute Error (MAE)"""
        return np.mean(np.sqrt(np.sum((a - b) ** 2, axis=1)))

    def _get_overlapping_xyz_parts_on_same_time_grid(
        self, out: np.ndarray, gt: np.ndarray
    ) -> tuple[np.ndarray, np.ndarray]:
        """
        Get overlapping parts of the trajectories on the same time grid.

        Args:
            out (np.ndarray): Estimated trajectory (Nx4 array: timestamp, x, y, z).
            gt (np.ndarray): Ground truth trajectory (Nx4 array: timestamp, x, y, z).

        Returns:
            tuple: (interpolated_est, gt_part) where:
                - interpolated_est: Estimated trajectory interpolated to the ground truth time grid.
                - gt_part: Ground truth trajectory overlapping with the estimated trajectory.
        """
        gt_t = gt[:, 0]
        out_t = out[:, 0]
        min_t = max(np.min(out_t), np.min(gt_t))
        max_t = min(np.max(out_t), np.max(gt_t))
        gt_part = gt[(gt_t >= min_t) & (gt_t <= max_t), :]
        out_part = np.hstack(
            [np.interp(gt_part[:, 0], out_t, out[:, i])[:, np.newaxis] for i in range(out.shape[1])]
        )
        return out_part[:, 1:], gt_part[:, 1:]

    def _align_to_gt(
        self,
        out: np.ndarray,
        gt: np.ndarray,
        rel_align_time: float = -1,
        fix_origin: bool = False,
        align3d: bool = True,
        fix_scale: bool = True,
    ) -> np.ndarray:
        """
        Align the estimated trajectory to the ground truth using Procrustes/Wahba SVD solution.

        Args:
            out (np.ndarray): Estimated trajectory (Nx4 array: timestamp, x, y, z).
            gt (np.ndarray): Ground truth trajectory (Nx4 array: timestamp, x, y, z).
            rel_align_time (float): Relative time for alignment (e.g., 1/3 of the session length).
            fix_origin (bool): Whether to fix the origin during alignment.
            align3d (bool): Whether to perform 3D alignment.
            fix_scale (bool): Whether to fix the scale during alignment.

        Returns:
            np.ndarray: Aligned estimated trajectory.
        """
        if len(out) <= 0 or len(gt) <= 0:
            return out

        out_part, gt_part = self._get_overlapping_xyz_parts_on_same_time_grid(out, gt)
        if out_part.shape[0] <= 0:
            return out

        if fix_origin:
            gt_ref = gt_part[0, :]
            out_ref = out_part[0, :]
        else:
            gt_ref = np.mean(gt_part, axis=0)
            out_ref = np.mean(out_part, axis=0)

        if align3d:
            if rel_align_time > 0:
                # Partial 3D align, not very well tested, use with caution
                t = int(len(out[:, 0]) * rel_align_time)
                if out_part.shape[0] > t and t > 0:
                    out_part = out_part[:t, :]
                    gt_part = gt_part[:t, :]

            out_xyz = (out_part - out_ref).transpose()
            gt_xyz = (gt_part - gt_ref).transpose()

            if out_xyz.shape[1] <= 0:
                return out

            if fix_scale:
                scale = 1
            else:
                get_scale = lambda xyz: np.mean(np.sqrt(np.sum(xyz**2, axis=0)))
                scale = min(get_scale(gt_xyz) / max(get_scale(out_xyz), 1e-5), 100)

            # Procrustes / Wahba SVD solution
            B = np.dot(gt_xyz, scale * out_xyz.transpose())
            U, S, Vt = np.linalg.svd(B)
            R = np.dot(U, Vt)
            if np.linalg.det(R) < 0.0:
                flip = np.diag([1, 1, -1])
                R = np.dot(U, np.dot(flip, Vt))
            R *= scale

            aligned = out * 1
            aligned[:, 1:4] = np.dot(R, (out[:, 1:] - out_ref).transpose()).transpose() + gt_ref
            return aligned

        # Else align in 2D
        xy_to_complex = lambda arr: arr[:, 0] + 1j * arr[:, 1]
        gt_xy = xy_to_complex(gt_part - gt_ref)
        out_xy = xy_to_complex(out_part - out_ref)

        rot = 1
        if rel_align_time > 0.0:
            t = int(len(out[:, 0]) * rel_align_time)
            max_t = min(len(out_xy), len(gt_xy))
            if t < max_t and np.minimum(np.abs(gt_xy[t]), np.abs(out_xy[t])) > 1e-5:
                rot = gt_xy[t] / out_xy[t]
            else:
                rel_align_time = -1

        if rel_align_time <= 0:
            valid = np.minimum(np.abs(gt_xy), np.abs(out_xy)) > 1e-5
            if np.sum(valid) > 0:
                rot = gt_xy[valid] / out_xy[valid]
                rot = rot / np.abs(rot)
                rot = np.mean(rot)

        if fix_scale:
            rot = rot / np.abs(rot)

        align_xy = xy_to_complex(out[:, 1:] - out_ref) * rot
        aligned = out * 1
        aligned[:, 1:] -= out_ref
        aligned[:, 1] = np.real(align_xy)
        aligned[:, 2] = np.imag(align_xy)
        aligned[:, 1:] += gt_ref
        return aligned

    def _read_euroc_trajectory(self, euroc_file: str) -> np.ndarray:
        """
        Read a trajectory from a EuRoC format file and convert timestamps to relative seconds.

        Args:
            euroc_file (str): Path to the EuRoC format file.

        Returns:
            np.ndarray: Trajectory as a Nx4 array (timestamp, x, y, z).
        """
        data = np.genfromtxt(euroc_file, delimiter=",")
        timestamps = data[:, 0]
        xyz = data[:, 1:4]
        first_timestamp = timestamps[0]
        timestamps_relative_seconds = (timestamps - first_timestamp) / 1e9
        return np.column_stack((timestamps_relative_seconds, xyz))

    def _read_jsonl_trajectory(self, jsonl_file: str) -> np.ndarray:
        """
        Read a trajectory from a JSONL file.

        Args:
            jsonl_file (str): Path to the JSONL file.

        Returns:
            np.ndarray: Trajectory as a Nx4 array (timestamp, x, y, z).
        """
        trajectory = []
        with open(jsonl_file, "r") as f:
            for line in f:
                data = json.loads(line)
                timestamp = data["time"]
                x = data["position"]["x"]
                y = data["position"]["y"]
                z = data["position"]["z"]
                trajectory.append([timestamp, x, y, z])
        return np.array(trajectory)

    def calculate_metric(
        self, gt_file: str, est_file
    ) -> float:
        """
        Calculate the requested metric between ground truth and estimated trajectories.

        Args:
            gt_file (str): Path to the ground truth trajectory file (EuRoC format).
            est_file (str): Path to the estimated trajectory file (JSONL format).

        Returns:
            float: The calculated metric value.
        """
        # Load ground truth and estimated trajectories
        gt = self._read_euroc_trajectory(gt_file)
        est = self._read_jsonl_trajectory(est_file)

        # Align the estimated trajectory to the ground truth
        est_aligned = self._align_to_gt(est, gt, align3d=True)

        # Get overlapping parts of the trajectories
        est_aligned, gt_aligned = self._get_overlapping_xyz_parts_on_same_time_grid(est_aligned, gt)

        # Calculate the requested metric
        if self.metric == "MAE":
            return self._mean_absolute_error(gt_aligned, est_aligned)
        elif self.metric == "RMSE":
            return self._rmse(gt_aligned, est_aligned)
        elif self.metric == "average":
            mae = self._mean_absolute_error(gt_aligned, est_aligned)
            rmse = self._rmse(gt_aligned, est_aligned)
            return (mae + rmse) / 2
        else:
            raise ValueError(f"Invalid metric: {self.metric}. Must be 'MAE', 'RMSE', or 'average'.")
