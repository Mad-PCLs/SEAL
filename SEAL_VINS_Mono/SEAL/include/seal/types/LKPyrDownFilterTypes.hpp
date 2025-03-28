#pragma once

#include <string>
#include <stdexcept>

enum class LKPyrDownFilterType {
    GAUSSIAN_5x5,
    GAUSSIAN_3x3,
    MEDIAN_3x3,
    BOX_3x3,
    BOX_2x2,
    DIRECT_SUBSAMPLE
};

LKPyrDownFilterType getLKPyrDownFilterTypeFromString(
    const std::string& name
);

std::string getLKPyrDownFilterTypeAsString(
    LKPyrDownFilterType type
);
