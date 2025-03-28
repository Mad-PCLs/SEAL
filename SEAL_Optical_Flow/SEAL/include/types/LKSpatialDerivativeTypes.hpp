#pragma once

#include <string>
#include <stdexcept>

enum class LKSpatialDerivativeType {
    SCHARR,
    SOBEL,
    BINARIZED,
    SCHARR_SCALED
};

LKSpatialDerivativeType getLKSpatialDerivativeTypeFromString(
    const std::string& name
);

std::string getLKSpatialDerivativeTypeAsString(
    LKSpatialDerivativeType type
);
