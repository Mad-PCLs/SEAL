#include "temporal_processing/TemporalProcessor.hpp"

TemporalProcessor::TemporalProcessor(
    DenoiserType denoiser_type,
    int edge_threshold
) : 
denoiser_type(denoiser_type),
edge_threshold(edge_threshold) { }

void TemporalProcessor::denoise(cv::Mat& img) {
    median_filter(img, denoiser_type);
}

void TemporalProcessor::edge_filter(cv::Mat& img) {
    rl_fast_edge_filter_wide(img, edge_threshold);
}
