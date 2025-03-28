#pragma once

#include <sstream>
#include <string>
#include <stdexcept>

#include "SEALProcessor.hpp"
#include "SEALPowerEmulator.hpp"

extern SEALProcessor* SEAL;
extern SEALPowerEmulator* SEAL_PEmu;


void SEALinit(
    bool use_seal,
    std::string paramFile,
    bool printConfig
);
void SEALclean();


void SEAL_PEmu_init(
    const std::string& emulator_mode,
    const std::string& storagePath,
    const std::string& sequencePath,
    char *argv[]
);
void SEAL_PEmu_clean();
