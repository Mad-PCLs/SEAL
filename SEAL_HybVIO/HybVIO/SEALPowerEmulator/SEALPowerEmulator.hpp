#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>

#include "../../src/tracker/track.hpp"

/* Define the searializers that will be used by cereal. */
namespace cereal
{
    template <class Archive>
    void serialize(Archive &archive, tracker::Feature::Point &point) {
        archive(point.x, point.y);
    }

    template <class Archive>
    void serialize(Archive &archive, tracker::Feature &feature) {
        archive(feature.id, feature.status, feature.points, feature.depth);
    }
}

class SEALPowerEmulator {
    public:
        enum class Mode { VANILLA, STORE, LOAD };
    
        SEALPowerEmulator(
            Mode mode,
            const std::string& basePath,
            const std::string& sequenceName,
            const std::string& execCommand
        );
    
        void storeOpticalFlow(
            const std::vector<tracker::Feature::Point>& prevCorners,
            const std::vector<tracker::Feature::Point>& corners,
            const std::vector<tracker::Feature::Point>& flowCorners,
            const std::vector<tracker::Feature>& tracks,
            const std::vector<tracker::Feature::Status>& trackStatus
        );
    
        bool loadOpticalFlow(
            std::vector<tracker::Feature::Point>& prevCorners,
            std::vector<tracker::Feature::Point>& corners,
            std::vector<tracker::Feature::Point>& flowCorners,
            std::vector<tracker::Feature>& tracks,
            std::vector<tracker::Feature::Status>& trackStatus
        );
    
        Mode getMode() const { return operationMode; }
    
    private:
        std::string basePath;
        std::filesystem::path sequencePath;
        Mode operationMode;
        int curr_id;
        
        void createFolders();
        void writeLogFile(const std::string& execCommand);
        std::string getFilename() const;
    };
