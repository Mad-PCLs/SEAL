#include "temporal_processor_node.hpp"

ImageProcessor::ImageProcessor(
    ros::NodeHandle &nh,
    std::string seal_params_file
) {
    SEAL = new SEALProcessor(seal_params_file);

    // Not sure about the queue_size arguments:
    // http://docs.ros.org/en/kinetic/api/roscpp/html/classros_1_1NodeHandle.html
    pub_ = nh.advertise<sensor_msgs::Image>(
        "/cam0/image_processed",
        1000
    );
    sub_ = nh.subscribe(
        "/cam0/image_raw",
        1000,
        &ImageProcessor::imgCallback,
        this
    );
}

void ImageProcessor::imgCallback(const sensor_msgs::ImageConstPtr& msg) {
    ROS_DEBUG("pre-processing raw image %d", img_id);
    // Convert ROS image to OpenCV format
    cv_bridge::CvImageConstPtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(
            msg,
            sensor_msgs::image_encodings::MONO8
        );
    }
    catch (cv_bridge::Exception& e) {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }
    
    // Process the image (apply any transformations here)
    cv::Mat processed_img = cv_ptr->image;
    SEAL->temporal_process(processed_img);
    ROS_DEBUG("pre-processing done for image %d", img_id);

    // Convert processed image back to ROS format
    sensor_msgs::ImagePtr processed_msg = cv_bridge::CvImage(
        std_msgs::Header(),
        "mono8",
        processed_img
    ).toImageMsg();
    processed_msg->header = msg->header;
    
    // Publish processed image
    pub_.publish(processed_msg);
    ROS_DEBUG("pre-processed image %d published!", img_id);
    img_id++;
}

ImageProcessor::~ImageProcessor() {
    delete SEAL;
    SEAL = nullptr;
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "seal_temporal_processor");
    ros::NodeHandle nh;
    std::string seal_params_file;
    if (!nh.getParam("/seal_temporal_processor/seal_params_file", seal_params_file)) {
        ROS_ERROR("Failed to get YAML file with params: 'seal_params_file.yaml'");
        return -1;
    }
    ImageProcessor img_processor(nh, seal_params_file);
    ros::spin();
    return 0;
}
