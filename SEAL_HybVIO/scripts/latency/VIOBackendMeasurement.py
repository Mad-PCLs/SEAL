from dataclasses import dataclass

@dataclass
class VIOBackendMeasurements:
    doRansac2: float = 0.0
    doRansac5: float = 0.0
    trackerVisualUpdate: float = 0.0
    KFpredict: float = 0.0

    def __add__(
        self,
        other: "VIOBackendMeasurements"
    ) -> "VIOBackendMeasurements":
        return VIOBackendMeasurements(
            doRansac2=self.doRansac2 + other.doRansac2,
            doRansac5=self.doRansac5 + other.doRansac5,
            trackerVisualUpdate=self.trackerVisualUpdate + other.trackerVisualUpdate,
            KFpredict=self.KFpredict + other.KFpredict,
        )

    def __truediv__(
        self,
        divisor: float
    ) -> "VIOBackendMeasurements":
        return VIOBackendMeasurements(
            doRansac2=self.doRansac2 / divisor,
            doRansac5=self.doRansac5 / divisor,
            trackerVisualUpdate=self.trackerVisualUpdate / divisor,
            KFpredict=self.KFpredict / divisor,
        )
