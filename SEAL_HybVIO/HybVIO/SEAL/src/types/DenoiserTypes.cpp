#include "types/DenoiserTypes.hpp"

DenoiserType getDenoiserTypeFromString(const std::string& name) {
    if (name == "THREE_PIX_MEDIAN")
        return DenoiserType::THREE_PIX_MEDIAN;
    if (name == "FIVE_PIX_MEDIAN")
        return DenoiserType::FIVE_PIX_MEDIAN;
    if (name == "THREE_BY_THREE_MEDIAN")
        return DenoiserType::THREE_BY_THREE_MEDIAN;

    throw std::invalid_argument("Invalid denoiser type: " + name);
}

std::string getDenoiserTypeAsString(DenoiserType type) {
    switch (type) {
        case DenoiserType::THREE_PIX_MEDIAN:
            return "THREE_PIX_MEDIAN";
        case DenoiserType::FIVE_PIX_MEDIAN:
            return "FIVE_PIX_MEDIAN";
        case DenoiserType::THREE_BY_THREE_MEDIAN:
            return "THREE_BY_THREE_MEDIAN";
    }

    throw std::invalid_argument("Invalid DenoiserType enum value.");
}
