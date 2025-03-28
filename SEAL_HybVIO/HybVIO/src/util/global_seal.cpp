#include "global_seal.hpp"

namespace fs = std::filesystem;

/* Global Variables */
SEALProcessor* SEAL = nullptr;
SEALPowerEmulator* SEAL_PEmu = nullptr;

/* Helper functions for SEAL */
void SEALinit(
    bool use_seal,
    std::string paramFile,
    bool printConfig
) {
    SEALConfig config;
    if (!paramFile.empty()) {
        config = SEALConfigBuilder()
                    .loadFromYAML(paramFile)
                    .build();
    }
    else {
        config = SEALConfigBuilder()
                    .setHighLevelGuards(false, false, false, false, false)
                    .build();
    }

    SEAL = new SEALProcessor(config);

    if (printConfig)
        std::cout << config << std::endl;
}

void SEALclean() {
    delete SEAL;
    SEAL = nullptr;
}

/* Helper functions for SEALPowerEmulator */
void SEAL_PEmu_init(
    const std::string& emulator_mode,
    const std::string& storagePath,
    const std::string& sequencePath,
    char *argv[]
) {
    /* Convert emulator_mode string to enum */
    SEALPowerEmulator::Mode mode;
    if (emulator_mode == "load") {
        mode = SEALPowerEmulator::Mode::LOAD;
    }
    else if (emulator_mode == "store") {
        mode = SEALPowerEmulator::Mode::STORE;
    }
    else if (emulator_mode == "vanilla" || emulator_mode.empty()) {
        mode = SEALPowerEmulator::Mode::VANILLA;
    }
    else {
        throw std::invalid_argument(
            "IN SEAL_PEmu_init, the emulator_mode argument"\
            " should be one of {load, store, vanilla}." \
            " Unknown emulator_mode: " + emulator_mode
        );
    }

    /* Convert argv to the string used to execute the binary */
    // Ignoring argc, since argv is null-terminated.
    // https://stackoverflow.com/a/11020198
    std::ostringstream full_command;
    int i = 0;

    // Iterate through argv[] until we hit nullptr
    while (argv[i] != nullptr) {
        full_command << argv[i] << " ";
        ++i;
    }

    /* Extract the folder name of the sequence */
    fs::path seqPath(sequencePath);
    // Check if the path is a directory
    if (!fs::is_directory(seqPath)) {
        throw std::invalid_argument("Path is not a folder: " + seqPath.string());
    }
    // Remove trailing slashes (if any) to ensure the last component is the folder name
    if (!seqPath.has_filename()) {
        seqPath = seqPath.parent_path();
    }

    SEAL_PEmu = new SEALPowerEmulator(
        mode,
        storagePath,
        seqPath.filename().string(),
        full_command.str()
    );
}

void SEAL_PEmu_clean() {
    delete SEAL_PEmu;
    SEAL_PEmu = nullptr;
}
