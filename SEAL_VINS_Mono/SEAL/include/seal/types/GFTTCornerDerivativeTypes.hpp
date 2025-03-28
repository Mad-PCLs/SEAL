#pragma once

#include <string>
#include <stdexcept>

enum class GFTTCornerDerivativeType {
    ORIGINAL,
    BINARIZED
};

GFTTCornerDerivativeType getGFTTCornerDerivativeTypeFromString(
    const std::string& name
);

std::string getGFTTCornerDerivativeTypeAsString(
    GFTTCornerDerivativeType type
);
