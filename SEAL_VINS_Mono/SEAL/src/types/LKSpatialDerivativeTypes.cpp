#include "types/LKSpatialDerivativeTypes.hpp"

LKSpatialDerivativeType getLKSpatialDerivativeTypeFromString(
    const std::string& name
) {
    if (name == "SCHARR")
        return LKSpatialDerivativeType::SCHARR;
    if (name == "SOBEL")
        return LKSpatialDerivativeType::SOBEL;
    if (name == "BINARIZED")
        return LKSpatialDerivativeType::BINARIZED;
    if (name == "SCHARR_SCALED")
        return LKSpatialDerivativeType::SCHARR_SCALED;

    throw std::invalid_argument("Invalid LK spatial derivative type: " + name);
}

std::string getLKSpatialDerivativeTypeAsString(
    LKSpatialDerivativeType type
) {
    switch (type) {
        case LKSpatialDerivativeType::SCHARR:
            return "SCHARR";
        case LKSpatialDerivativeType::SOBEL:
            return "SOBEL";
        case LKSpatialDerivativeType::BINARIZED:
            return "BINARIZED";
        case LKSpatialDerivativeType::SCHARR_SCALED:
            return "SCHARR_SCALED";
    }

    throw std::invalid_argument("Invalid LKSpatialDerivativeType enum value.");
}
