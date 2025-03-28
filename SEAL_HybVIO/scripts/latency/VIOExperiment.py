from dataclasses import dataclass

from .VIOFrontendMeasurements import VIOFrontendMeasurements
from .VIOBackendMeasurement import VIOBackendMeasurements

@dataclass
class VIOExperiment:
    frontend: VIOFrontendMeasurements
    backend: VIOBackendMeasurements

    def __add__(self, other: "VIOExperiment") -> "VIOExperiment":
        return VIOExperiment(
            frontend=self.frontend + other.frontend,
            backend=self.backend + other.backend,
        )

    def __truediv__(self, divisor: float) -> "VIOExperiment":
        return VIOExperiment(
            frontend=self.frontend / divisor,
            backend=self.backend / divisor,
        )
