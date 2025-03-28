#pragma once

#include <string>
#include <stdexcept>

enum class DenoiserType {
    THREE_PIX_MEDIAN,
    FIVE_PIX_MEDIAN,
    THREE_BY_THREE_MEDIAN
};

DenoiserType getDenoiserTypeFromString(const std::string& name);

std::string getDenoiserTypeAsString(DenoiserType type);
