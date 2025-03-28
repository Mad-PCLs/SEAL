#include "types/LKPyrDownFilterTypes.hpp"

LKPyrDownFilterType getLKPyrDownFilterTypeFromString(
    const std::string& name
) {
    if (name == "GAUSSIAN_5x5")
        return LKPyrDownFilterType::GAUSSIAN_5x5;
    if (name == "GAUSSIAN_3x3")
        return LKPyrDownFilterType::GAUSSIAN_3x3;
    if (name == "MEDIAN_3x3")
        return LKPyrDownFilterType::MEDIAN_3x3;
    if (name == "BOX_3x3")
        return LKPyrDownFilterType::BOX_3x3;
    if (name == "BOX_2x2")
        return LKPyrDownFilterType::BOX_2x2;
    if (name == "DIRECT_SUBSAMPLE")
        return LKPyrDownFilterType::DIRECT_SUBSAMPLE;

    throw std::invalid_argument("Invalid LK pyramid downsample filter type: " + name);
}

std::string getLKPyrDownFilterTypeAsString(
    LKPyrDownFilterType type
) {
    switch (type) {
        case LKPyrDownFilterType::GAUSSIAN_5x5:
            return "GAUSSIAN_5x5";
        case LKPyrDownFilterType::GAUSSIAN_3x3:
            return "GAUSSIAN_3x3";
        case LKPyrDownFilterType::MEDIAN_3x3:
            return "MEDIAN_3x3";
        case LKPyrDownFilterType::BOX_3x3:
            return "BOX_3x3";
        case LKPyrDownFilterType::BOX_2x2:
            return "BOX_2x2";
        case LKPyrDownFilterType::DIRECT_SUBSAMPLE:
            return "DIRECT_SUBSAMPLE";
    }

    throw std::invalid_argument("Invalid LKPyrDownFilterType enum value.");
}
