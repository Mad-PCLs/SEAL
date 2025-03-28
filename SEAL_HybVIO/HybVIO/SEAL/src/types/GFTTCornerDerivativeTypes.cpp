#include "types/GFTTCornerDerivativeTypes.hpp"

GFTTCornerDerivativeType getGFTTCornerDerivativeTypeFromString(
    const std::string& name
) {
    if (name == "ORIGINAL")
        return GFTTCornerDerivativeType::ORIGINAL;
    if (name == "BINARIZED")
        return GFTTCornerDerivativeType::BINARIZED;

    throw std::invalid_argument("Invalid GFTT corner derivative type: " + name);
}

std::string getGFTTCornerDerivativeTypeAsString(
    GFTTCornerDerivativeType type
) {
    switch (type) {
        case GFTTCornerDerivativeType::ORIGINAL:
            return "ORIGINAL";
        case GFTTCornerDerivativeType::BINARIZED:
            return "BINARIZED";
    }

    throw std::invalid_argument("Invalid GFTTCornerDerivativeType enum value.");
}
