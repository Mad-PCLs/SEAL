from typing import Literal, TypedDict, List, Optional
from dataclasses import dataclass

# Define types for vision kind (stereo, cam0, cam1)
VisionKind = Literal["stereo", "cam0", "cam1"]

# Define camera parameters
@dataclass
class CameraParameters:
    focalLengthX: float
    focalLengthY: float
    principalPointX: float
    principalPointY: float

# Define a single frame in the dataset
@dataclass
class Frame:
    cameraInd: int
    cameraParameters: CameraParameters
    time: float
    number: Optional[int] = None  # Only for monocular frames

@dataclass
class Position:
    x: float
    y: float
    z: float

@dataclass
class Orientation:
    w: float
    x: float
    y: float
    z: float

# Define ground truth data
@dataclass
class GroundTruth:
    position: Position
    orientation: Orientation

# Define sensor data (gyroscope or accelerometer)
@dataclass
class SensorData:
    type: Literal["gyroscope", "accelerometer"]
    values: List[float]

# Define a single entry in the JSONL file
@dataclass
class DatasetEntry:
    time: float
    number: Optional[int] = None
    frames: Optional[List[Frame]] = None       # Only for vision data
    groundTruth: Optional[GroundTruth] = None  # Only for ground truth data
    sensor: Optional[SensorData] = None        # Only for IMU data
