#include <string>

#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include "seal/SEALProcessor.hpp"

class ImageProcessor
{
    public:
        ImageProcessor(ros::NodeHandle &nh, std::string seal_params_file);
        ~ImageProcessor();

    private:
        ros::Publisher pub_;
        ros::Subscriber sub_;

        SEALProcessor* SEAL;

        int img_id = 0;

        void imgCallback(const sensor_msgs::ImageConstPtr &img_msg);
};
