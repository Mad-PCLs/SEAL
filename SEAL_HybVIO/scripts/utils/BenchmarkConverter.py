# The following class handles the conversion of the raw EuRoC dataset
# to the benchmark format that is required by HybVIO.
# It is heavily based on: https://github.com/AaltoML/vio_benchmark/blob/main/convert/euroc_to_benchmark.py

import os
from pathlib import Path
import subprocess
from typing import List, Dict, Tuple, Optional
import dataclasses

import csv
import json
import yaml

import numpy as np

from utils.jsonl_types import (
    VisionKind,
    CameraParameters,
    Frame,
    Position,
    Orientation,
    GroundTruth,
    SensorData,
    DatasetEntry
)

class BenchmarkConverter:
    def __init__(
        self,
        dataset: Dict[str, str],
        raw_dir: str,
        output_dir: str
    ):
        self.dataset = dataset
        self.raw_dir = raw_dir
        self.output_dir = output_dir
        self.to_seconds = 1000 * 1000 * 1000

    def convert_to_benchmark_format(self):
        name = self.dataset['name']
        Path(self.output_dir).mkdir(parents=True, exist_ok=True)

        # Read ground truth and IMU data
        output, t0 = self._read_ground_truth_and_imu()

        self._make_dataset(output, name, t0, "stereo")

        print(f"{name} conversion to benchmark format complete.")

    def _read_ground_truth_and_imu(self) -> Tuple[List[DatasetEntry], float]:
        output: List[DatasetEntry] = []
        t0: Optional[float] = None

        # Read ground truth
        with open(os.path.join(self.raw_dir, 'mav0/state_groundtruth_estimate0/data.csv')) as csvfile:
            csvreader = csv.reader(csvfile, delimiter=',')
            next(csvreader)  # Skip header
            for row in csvreader:
                t = int(row[0]) / self.to_seconds
                if t0 is None:
                    t0 = t
                timestamp = t - t0
                ground_truth = GroundTruth(
                    position=Position(
                        x=float(row[1]),
                        y=float(row[2]),
                        z=float(row[3])
                    ),
                    orientation=Orientation(
                        w=float(row[4]),
                        x=float(row[5]),
                        y=float(row[6]),
                        z=float(row[7])
                    )
                )
                output.append(
                    DatasetEntry(
                        groundTruth=ground_truth,
                        time=timestamp        
                    )
                )

        # Read IMU data
        with open(os.path.join(self.raw_dir, 'mav0/imu0/data.csv')) as csvfile:
            csvreader = csv.reader(csvfile, delimiter=',')
            next(csvreader)  # Skip header
            for row in csvreader:
                timestamp = int(row[0]) / self.to_seconds - t0
                gyroscope = SensorData(
                    type="gyroscope",
                    values=[
                        float(row[1]), # x
                        float(row[2]), # y
                        float(row[3])  # z
                    ]
                )
                accelerometer = SensorData(
                    type="accelerometer",
                    values=[
                        float(row[4]), # x
                        float(row[5]), # y
                        float(row[6])  # z
                    ]
                )
                output.append(
                    DatasetEntry(
                        sensor=gyroscope,
                        time=timestamp
                    )
                )
                output.append(
                    DatasetEntry(
                        sensor=accelerometer,
                        time=timestamp
                    )
                )

        return output, t0

    def _make_dataset(
        self,
        output: List[DatasetEntry],
        name: str,
        t0: float,
        kind: VisionKind
    ):
        assert kind in ["stereo", "cam0", "cam1"]
        stereo = kind == "stereo"

        if not stereo:
            self.output_dir = self.output_dir + f"-{kind}"
        Path(self.output_dir).mkdir(parents=True, exist_ok=True)

        # Get camera parameters
        parameters0 = self._get_camera_parameters(
            os.path.join(self.raw_dir, "mav0/cam0/sensor.yaml")
        )
        parameters1: Optional[CameraParameters] = None
        if stereo:
            parameters1 = self._get_camera_parameters(
                os.path.join(self.raw_dir, "mav0/cam1/sensor.yaml")
            )

        # Process timestamps and frames
        timestamps = self._process_timestamps(kind, stereo)

        self._write_jsonl_file(
            output=output,
            timestamps=timestamps,
            stereo=stereo,
            parameters0=parameters0,
            parameters1=parameters1,
            t0=t0
        )

        self._compile_videos(
            output=output,
            stereo=stereo,
            kind=kind
        )

        # Write additional files (info.json, parameters.txt)
        self._write_additional_files(
            stereo=stereo,
            kind=kind
        )

    def _get_camera_parameters(
        self,
        filename: str
    ) -> CameraParameters:
        with open(filename) as f:
            intrinsics = yaml.load(f, Loader=yaml.FullLoader)["intrinsics"]
            return CameraParameters(
                focalLengthX=intrinsics[0],
                focalLengthY=intrinsics[1],
                principalPointX=intrinsics[2],
                principalPointY=intrinsics[3]
            )

    def _process_timestamps(
        self,
        kind: VisionKind,
        stereo: bool
    ) -> List[float]:
        timestamps = []
        if stereo:
            dir0 = os.path.join(self.raw_dir, "mav0/cam0/data")
            dir1 = os.path.join(self.raw_dir, "mav0/cam1/data")
            timestamps0 = [f for f in os.listdir(dir0)]
            timestamps1 = [f for f in os.listdir(dir1)]
            for t in timestamps0:
                if t in timestamps1:
                    timestamps.append(int(os.path.splitext(t)[0]) / self.to_seconds)
                else:
                    os.rename(os.path.join(dir0, t), os.path.join(dir0, t + "_hdn"))
                    print(f"{t} not found in cam1, ignoring")
            for t in timestamps1:
                if t not in timestamps0:
                    os.rename(os.path.join(dir1, t), os.path.join(dir1, t + "_hdn"))
                    print(f"{t} not found in cam0, ignoring")
        else:
            dir = os.path.join(self.raw_dir, f"mav0/{kind}/data")
            timestamps = [int(os.path.splitext(f)[0]) / self.to_seconds for f in os.listdir(dir)]
        return sorted(timestamps)

    def _compile_videos(
        self,
        output: List[DatasetEntry],
        stereo: bool,
        kind: VisionKind,
    ):
        # Generate JSONL
        output = sorted(output, key=lambda row: row.time)
        with open(os.path.join(self.output_dir, "data.jsonl"), "w") as f:
            for obj in output:
                f.write(
                    json.dumps(
                        dataclasses.asdict(obj, dict_factory=lambda x: {k: v for (k, v) in x if v is not None}),
                        separators=(',', ':')
                    ) + "\n"
                )

        # Generate videos
        if stereo:
            self._convert_video(
                files=os.path.join(self.raw_dir, "mav0/cam0/data/*.png"),
                output=os.path.join(self.output_dir, "data.mp4"),
                fps=20
            )
            self._convert_video(
                files=os.path.join(self.raw_dir, "mav0/cam1/data/*.png"),
                output=os.path.join(self.output_dir, "data2.mp4"),
                fps=20
            )
        else:
            self._convert_video(
                files=os.path.join(self.raw_dir, f"mav0/{kind}/data/*.png"),
                output=os.path.join(self.output_dir, "data.mp4"),
                fps=20
            )

    def _write_jsonl_file(
        self,
        output: List[DatasetEntry],
        timestamps: List[float],
        stereo: bool,
        parameters0: CameraParameters,
        parameters1: Optional[CameraParameters],
        t0: float
    ):
        """
        Constructs frames, appends them to the output list, and writes the JSONL file.
        """
        # Construct frames and append to output
        number = 0
        for timestamp in timestamps:
            t = timestamp - t0
            curr_entry = DatasetEntry(
                number=number,
                time=t,
                frames=[]
            )
            if stereo:
                frame0 = Frame(
                    cameraInd=0,
                    cameraParameters=parameters0,
                    time=t
                )
                frame1 = Frame(
                    cameraInd=1,
                    cameraParameters=parameters1,  # parameters1 is guaranteed to exist if stereo is True
                    time=t
                )
                curr_entry.frames = [frame0, frame1]
            else:
                frame0 = Frame(
                    cameraInd=0,
                    cameraParameters=parameters0,
                    number=number,  # Only for monocular frames
                    time=t
                )
                curr_entry.frames = [frame0]
            output.append(curr_entry)
            number += 1

        # Sort output by time and write JSONL file
        output = sorted(output, key=lambda row: row.time)
        with open(os.path.join(self.output_dir, "data.jsonl"), "w") as f:
            for obj in output:
                f.write(
                    json.dumps(
                        dataclasses.asdict(obj, dict_factory=lambda x: {k: v for (k, v) in x if v is not None}),
                        separators=(',', ':')
                    ) + "\n"
                )

    def _write_additional_files(
        self,
        stereo: bool,
        kind: VisionKind
    ):
        # Write info.json
        with open(os.path.join(self.output_dir, "info.json"), "w") as f:
            f.write(json.dumps({"tags": ["euroc"]}, indent=4))

        # Write parameters.txt
        with open(os.path.join(self.output_dir, "parameters.txt"), "w") as f, \
             open(os.path.join(self.raw_dir, 'mav0/cam0/sensor.yaml')) as s0, \
             open(os.path.join(self.raw_dir, 'mav0/cam1/sensor.yaml')) as s1:

            sensor0 = yaml.load(s0, Loader=yaml.FullLoader)
            sensor1 = yaml.load(s1, Loader=yaml.FullLoader)

            # Inverses of sensor.yaml matrices, column-major
            m0 = ",".join(str(i) for i in np.linalg.inv(np.array(sensor0["T_BS"]["data"]).reshape((4, 4))).T.reshape(16))
            m1 = ",".join(str(i) for i in np.linalg.inv(np.array(sensor1["T_BS"]["data"]).reshape((4, 4))).T.reshape(16))

            if stereo:
                i0 = sensor0["intrinsics"]
                i1 = sensor1["intrinsics"]
                d0 = sensor0["distortion_coefficients"]
                d1 = sensor1["distortion_coefficients"]
            elif kind == "cam0":
                i0 = sensor0["intrinsics"]
                d0 = sensor0["distortion_coefficients"]
            elif kind == "cam1":
                i0 = sensor1["intrinsics"]
                d0 = sensor1["distortion_coefficients"]
                m0 = m1

            f.write(f"focalLengthX {i0[0]};focalLengthY {i0[1]};principalPointX {i0[2]};principalPointY {i0[3]};\n"
                    f"distortionCoeffs {d0[0]},{d0[1]},{d0[2]};\n")
            if stereo:
                f.write(f"secondFocalLengthX {i1[0]};secondFocalLengthY {i1[1]};\n"
                        f"secondPrincipalPointX {i1[2]};secondPrincipalPointY {i1[3]};\n"
                        f"secondDistortionCoeffs {d1[0]},{d1[1]},{d1[2]};\n")
            f.write("rot 0;\nvideoRotation NONE;\n"
                    f"imuToCameraMatrix {m0};\n")
            if stereo:
                f.write(f"secondImuToCameraMatrix {m1};\n"
                        "matchStereoIntensities true;\n")

    @staticmethod
    def _convert_video(
        files: str,
        output: str,
        fps: int
    ):
        subprocess.run(
            [
                "ffmpeg",
                "-y", "-r", str(fps),
                "-f", "image2",
                "-pattern_type", "glob",
                "-i", files,
                "-c:v", "libx264",
                "-preset", "veryslow",
                "-crf", "0",
                "-vf", "format=yuv420p",
                "-an", output
            ],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
