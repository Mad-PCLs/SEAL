#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <sstream>
#include <iomanip>

#include "SEALProcessor.hpp"

#define FPS_ORIGINAL 200
#define EPS 1e-5

#define THRESH_MIN 20 // Minimum edge threshold value, inclusive
#define THRESH_MAX 45 // Maximum edge threshold value, inclusive

#define SEAL_PARAMS_PATH "../SEAL/seal_params.yaml"
#define DATA_GT_PATH "../data/hd1k_flow_gt/flow_occ/"
#define DATA_UNC_PATH "../data/hd1k_flow_unc/flow_unc/"
#define DATA_FRAMES_PATH "../data/hd1k_input/image_2/"
#define OUTPUT_FILE "../output/epe_errors.csv"

typedef std::pair<cv::Mat, cv::Mat> GtFlowPair;

namespace fs = std::filesystem;

std::vector<std::string> getSortedFiles(
    const int seq_id,
    const std::string& folder,
    const std::string& extension = ".png"
) {
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << seq_id;
    std::string seq_str = ss.str();

    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(folder)) {
        if ((entry.path().extension() == extension)
            && (entry.path().filename().string().rfind(seq_str, 0) == 0)
        ) {
            files.push_back(entry.path().string());
        }
    }
    // Ensure frames are processed in order
    std::sort(files.begin(), files.end());
    return files;
}

void computeSparseOpticalFlow(
    const cv::Mat& frame1,
    const cv::Mat& frame2,
    std::vector<cv::Point2f>& keypoints,
    std::vector<cv::Point2f>& nextPts,
    std::vector<uchar>& status,
    SEALProcessor& seal
) {
    assert(keypoints.empty() & nextPts.empty() & status.empty());
    std::vector<float> err;
    std::vector<float> cornersQuality;

    // Detect feature points in first frame
    if (seal.isKeypointDetectorOn()) {
        seal.detect_keypoints(frame1, keypoints, cornersQuality, cv::noArray());
    }
    else {
        cv::goodFeaturesToTrack(frame1, keypoints, 1000, 0.01, 1, cv::noArray(), 3);
    }

    // Track the points in the next frame using Lucas-Kanade optical flow
    if (seal.isKeypointTrackerOn()) {
        seal.track_keypoints(frame1, frame2, keypoints, nextPts, status, err);
    }
    else {
        cv::calcOpticalFlowPyrLK(frame1, frame2, keypoints, nextPts, status, err);
    }
}

double computeEPE(
    const std::vector<cv::Point2f>& keypoints,
    const std::vector<cv::Point2f>& nextPts,
    const std::vector<uchar>& status,
    const GtFlowPair& gt_pair,
    const cv::Mat& uncMat
) {
    double totalEPE = 0.0;
    int validCount = 0;

    cv::Mat gt_flow_vecs = gt_pair.first;
    cv::Mat valid_mat = gt_pair.second;

    for (size_t i = 0; i < keypoints.size(); i++) {
        // Process only valid tracked points
        if (status[i]) {
            int posx = static_cast<int>(keypoints[i].x);
            int posy = static_cast<int>(keypoints[i].y);

            if (
                valid_mat.at<uint8_t>(cv::Point(posx, posy)) == 0 ||
                uncMat.at<uint8_t>(cv::Point(posx, posy))
            ) {
                continue;
            }

            // https://stackoverflow.com/a/25644503
            cv::Vec2f gtVector = gt_flow_vecs.at<cv::Vec2f>(cv::Point(posx, posy));
            cv::Vec2f estVector(nextPts[i].x - keypoints[i].x, nextPts[i].y - keypoints[i].y);

            double dx = estVector[0] - gtVector[0];
            double dy = estVector[1] - gtVector[1];

            totalEPE += std::sqrt(dx * dx + dy * dy);
            validCount++;
        }
    }
    return (validCount > 0) ? (totalEPE / validCount) : 0.0;
}

/* Subsampling the dataset creates the question, which
 * ground truth file should we compare against?
 * Since the ground truth is just displacements we
 * need to add the displacements that occure during the
 * time interval between two consecutive frames in the
 * target fps.
 * We also need to update the valid channels accordingly.
 * The function returns first the Matrix with 2 channels,
 * where each (x, y) position corresponds to a 2d-vector
 * representing the flow at that specific pixel.
 * The second Matrix returned is the 2D tensor with the
 * validity bits.
 */
GtFlowPair getAccumulatedGroundTruth(
    std::vector<std::string> gtFiles,
    size_t start_idx,
    size_t end_idx
) {
    cv::Mat acc_flow;
    cv::Mat valid;
    // Notice that we ignore the last ground truth index
    // as instructed by the README.md.
    for (size_t i = start_idx; i < end_idx; i++) {
        cv::Mat flow_img = cv::imread(gtFiles[i], cv::IMREAD_UNCHANGED);
        cv::cvtColor(flow_img, flow_img, cv::COLOR_BGR2RGB);

        std::vector<cv::Mat> flow_channels;
        cv::split(flow_img, flow_channels);

        // Update validity matrix
        if (valid.empty()) {
            valid = (flow_channels[2] > 0);
        }
        else {
            valid &= (flow_channels[2] > 0);
        }

        // Update ground truth flow vectors
        cv::Mat curr_flow;
        std::vector<cv::Mat> flow_components = {flow_channels[0], flow_channels[1]};
        cv::merge(flow_components, curr_flow);
        curr_flow.convertTo(curr_flow, CV_32F);
        curr_flow = (curr_flow - pow(2, 15)) / 64.0;
        if (acc_flow.empty()) {
            acc_flow = curr_flow;
        }
        else {
            acc_flow += curr_flow;
        }
    }

    return std::make_pair(acc_flow, valid);
}

cv::Mat getAccumulatedUncertainty(
    std::vector<std::string> uncFiles,
    size_t start_idx,
    size_t end_idx
) {
    cv::Mat acc_unc;
    // Notice that we ignore the last ground truth index
    // as instructed by the README.md.
    for (size_t i = start_idx; i < end_idx; i++) {
        cv::Mat cur_unc = cv::imread(uncFiles[i], cv::IMREAD_UNCHANGED);
        cur_unc.convertTo(cur_unc, CV_32F, 1.0 / 256.0);

        // Based on the README.md, pixels with uncertainty > 0 are valid.
        if (acc_unc.empty()) {
            acc_unc = (cur_unc > 0);
        }
        else {
            acc_unc &= (cur_unc > 0);
        }
    }
    return acc_unc;
}

/* Calculate End-point Error for constant fps.
 * The reason we need the period specified, instead
 * of the fps, is that not all fps are achievable.
 * We have a constant fps for the dataset, hence each
 * frame is captured every 0.005 seconds, thus you can
 * achieve periods that are multiples of this period.
 * For example, to achieve 150 fps, you would need to
 * sample one frame every 1 / 150 = 0.00667, which is
 * obviously impossible to get.
 */
std::pair<double, int> get_epe_4_target_period(
    std::vector<std::string> frameFiles,
    std::vector<std::string> uncFiles,
    std::vector<std::string> gtFiles,
    float target_period,
    SEALProcessor& seal
) {
    // Make sure that we can actually achieve the target period
    assert(target_period > 0);
    float ffstep = target_period * FPS_ORIGINAL;
    int fstep = static_cast<int>(std::round(ffstep));
    assert(std::fabs(fstep - ffstep) < EPS);
    assert(frameFiles.size() - fstep > 0);

    int count = 0;
    double totalError = 0.0;
    for (size_t i = 0; i < frameFiles.size() - fstep; i++) {
        std::vector<cv::Point2f> keypoints;
        std::vector<cv::Point2f> nextPts;
        std::vector<uchar> status;

        // Load two dataset images, gt and uncertainties
        cv::Mat frame1 = cv::imread(frameFiles[i], cv::IMREAD_GRAYSCALE);
        cv::Mat frame2 = cv::imread(frameFiles[i + fstep], cv::IMREAD_GRAYSCALE);
        seal.temporal_process(frame1);
        seal.temporal_process(frame2);

        GtFlowPair gt_pair = getAccumulatedGroundTruth(gtFiles, i, i+fstep);
        cv::Mat uncMat = getAccumulatedUncertainty(uncFiles, i, i+fstep);

        if (
            frame1.empty()
            || frame2.empty()
            || gt_pair.first.empty()
            || gt_pair.first.empty()
            || uncMat.empty()
        ) {
            std::cerr << "Error loading images at index: " << i << std::endl;
            continue;
        }

        computeSparseOpticalFlow(frame1, frame2, keypoints, nextPts, status, seal);
        totalError += computeEPE(keypoints, nextPts, status, gt_pair, uncMat);
        count++;
    }

    return std::make_pair(totalError/count, count);
}

int main(int argc, char** argv) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);

    const std::string output_csv = OUTPUT_FILE;
    std::vector<int> edge_thresholds;

    for (int t = THRESH_MIN; t <= THRESH_MAX; ++t) edge_thresholds.push_back(t); // Push all thresholds in range
    edge_thresholds.push_back(THRESH_MAX+1); // Add Baseline case as the last column

    const cv::String keys =
        "{help h usage ? |       | Show help message}"
        "{seq_id         | -1    | Run all edge thresholds on this sequence only}"
        "{edge_thresh    | -1    | Run this edge threshold across all sequences}";

    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("Optical Flow EPE Evaluation Tool");

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    int seq_id = parser.get<int>("seq_id");
    int edge_thresh = parser.get<int>("edge_thresh");

    if (!parser.check()) {
        parser.printErrors();
        return -1;
    }

    // 36 sequences in total
    std::vector<std::vector<double>> all_epe_data(36, std::vector<double>(edge_thresholds.size(), -1));

    auto run_evaluation = [&](int sid, int t) -> double {
        SEALConfigBuilder builder = SEALConfigBuilder().loadFromYAML(SEAL_PARAMS_PATH);
        if (t == THRESH_MAX+1) {
            // For the last column, ie the baseline case, turn off SEAL Processor
            builder.setHighLevelGuards(false, false, false, false, false);
        } else {
            builder.setEdgeThreshold(t);
        }

        SEALProcessor seal(builder.build());

        auto gtFiles = getSortedFiles(sid, DATA_GT_PATH);
        auto uncFiles = getSortedFiles(sid, DATA_UNC_PATH);
        auto frameFiles = getSortedFiles(sid, DATA_FRAMES_PATH);

        auto res = get_epe_4_target_period(frameFiles, uncFiles, gtFiles, 1.0 / FPS_ORIGINAL, seal);
        return res.first;
    };

    if (seq_id >= 0 && edge_thresh == -1) {
        std::cout << "Running sequence " << seq_id << " across all thresholds:\n";
        for (size_t i = 0; i < edge_thresholds.size(); ++i) {
            double epe = run_evaluation(seq_id, edge_thresholds[i]);
            all_epe_data[seq_id][i] = epe;
            std::cout << "  Threshold " << (edge_thresholds[i] == THRESH_MAX+1 ? "Baseline" : std::to_string(edge_thresholds[i]))
                      << ": EPE = " << epe << "\n";
        }

    } else if (seq_id == -1 && edge_thresh >= THRESH_MIN && edge_thresh <= THRESH_MAX+1) {
        std::cout << "Running threshold " << (edge_thresh == THRESH_MAX+1 ? "Baseline" : std::to_string(edge_thresh))
                  << " across all sequences:\n";
        size_t t_idx = std::distance(edge_thresholds.begin(),
                                     std::find(edge_thresholds.begin(), edge_thresholds.end(), edge_thresh));

        for (int sid = 0; sid < 36; ++sid) {
            double epe = run_evaluation(sid, edge_thresh);
            all_epe_data[sid][t_idx] = epe;
            std::cout << "  Sequence " << sid << ": EPE = " << epe << "\n";
        }

    } else if (seq_id == -1 && edge_thresh == -1) {
        std::cout << "Running full sweep (all sequences across all thresholds):\n";
        for (int sid = 0; sid < 36; ++sid) {
            std::cout << "Sequence " << sid << ":\n";
            for (size_t i = 0; i < edge_thresholds.size(); ++i) {
                double epe = run_evaluation(sid, edge_thresholds[i]);
                all_epe_data[sid][i] = epe;
                std::cout << "  Threshold " << (edge_thresholds[i] == THRESH_MAX+1 ? "Baseline" : std::to_string(edge_thresholds[i]))
                          << ": EPE = " << epe << "\n";
            }
        }
    } else {
        std::cerr << "Invalid combination: both --seq_id and --edge_thresh provided. Please use only one.\n";
        return -1;
    }

    // Write CSV
    std::filesystem::create_directories(std::filesystem::path(output_csv).parent_path());
    std::ofstream out(output_csv);
    out << "Sequence";
    for (int t : edge_thresholds) {
        out << "," << (t == THRESH_MAX+1 ? "Baseline" : std::to_string(t));
    }
    out << "\n";

    for (int sid = 0; sid < 36; ++sid) {
        out << sid;
        for (double epe : all_epe_data[sid]) {
            if (epe >= 0)
                out << "," << epe;
            else
                out << ",";
        }
        out << "\n";
    }

    out.close();
    std::cout << "\nCSV summary written to " << output_csv << "\n";
    return 0;
}
