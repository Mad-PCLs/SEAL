from dataclasses import dataclass

@dataclass
class VIOFrontendMeasurements:
    findKeypoints: float = 0.0
    computeOpticalFlow: float = 0.0

    def __add__(
        self,
        other: "VIOFrontendMeasurements"
    ) -> "VIOFrontendMeasurements":
        return VIOFrontendMeasurements(
            findKeypoints=self.findKeypoints + other.findKeypoints,
            computeOpticalFlow=self.computeOpticalFlow + other.computeOpticalFlow,
        )

    def __truediv__(
        self,
        divisor: float
    ) -> "VIOFrontendMeasurements":
        return VIOFrontendMeasurements(
            findKeypoints=self.findKeypoints / divisor,
            computeOpticalFlow=self.computeOpticalFlow / divisor,
        )
