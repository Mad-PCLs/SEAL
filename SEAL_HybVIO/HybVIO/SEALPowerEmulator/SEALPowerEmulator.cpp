#include "SEALPowerEmulator.hpp"

namespace fs = std::filesystem;

SEALPowerEmulator::SEALPowerEmulator(
    Mode mode,
    const std::string& basePath,
    const std::string& sequenceName,
    const std::string& execCommand
) : 
    basePath(basePath),
    operationMode(mode),
    curr_id(0) 
{    
    sequencePath = fs::path(basePath) / sequenceName;

    if (operationMode != Mode::VANILLA && basePath.empty()) {
        throw std::invalid_argument(
            "Unspecified basePath for SEALPowerEmulator!"
        );
    }

    if (operationMode == Mode::STORE) {
        createFolders();
        writeLogFile(execCommand);
    }
    else if (operationMode == Mode::LOAD) {
        createFolders();
        writeLogFile(execCommand);
    }
}

void SEALPowerEmulator::createFolders() {
    try {
        if (!fs::exists(basePath)) {
            fs::create_directories(basePath);
        }
        
        if (!fs::exists(sequencePath)) {
            fs::create_directories(sequencePath);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating folders: " << e.what() << std::endl;
    }
}

void SEALPowerEmulator::writeLogFile(const std::string& execCommand) {
    fs::path logFilename = sequencePath / "run_log.txt";

    std::ofstream logFile(logFilename, std::ios::app);
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
        
        logFile << "==========================================\n";
        logFile << "Run Timestamp: " << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S") << "\n";
        logFile << "Execution Command: " << execCommand << "\n";
        logFile << "==========================================\n\n";

        logFile.close();
    } else {
        std::cerr << "Error: Could not open log file for writing." << std::endl;
    }
}

std::string SEALPowerEmulator::getFilename() const {
    return (sequencePath / (std::to_string(curr_id) + ".bin")).string();
}

void SEALPowerEmulator::storeOpticalFlow(
    const std::vector<tracker::Feature::Point>& prevCorners,
    const std::vector<tracker::Feature::Point>& corners,
    const std::vector<tracker::Feature::Point>& flowCorners,
    const std::vector<tracker::Feature>& tracks,
    const std::vector<tracker::Feature::Status>& trackStatus
) {
    if (operationMode != Mode::STORE) return;

    std::ofstream file(getFilename(), std::ios::binary);
    if (file.is_open()) {
        cereal::BinaryOutputArchive archive(file);
        archive(prevCorners, corners, flowCorners, tracks, trackStatus);
        curr_id++;
    } else {
        std::cerr << "Error: Could not open " << getFilename() << " for writing." << std::endl;
    }
}

bool SEALPowerEmulator::loadOpticalFlow(
    std::vector<tracker::Feature::Point>& prevCorners,
    std::vector<tracker::Feature::Point>& corners,
    std::vector<tracker::Feature::Point>& flowCorners,
    std::vector<tracker::Feature>& tracks,
    std::vector<tracker::Feature::Status>& trackStatus
) {
    if (operationMode != Mode::LOAD) return false;

    std::ifstream file(getFilename(), std::ios::binary);
    if (file.is_open()) {
        cereal::BinaryInputArchive archive(file);
        archive(prevCorners, corners, flowCorners, tracks, trackStatus);
        curr_id++;
        return true;
    } else {
        std::cerr << "Error: Could not open " << getFilename() << " for reading." << std::endl;
        return false;
    }
}
