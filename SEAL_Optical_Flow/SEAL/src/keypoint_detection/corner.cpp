// License: See the LICENSE in the header file.

#include "keypoint_detection/corner.hpp"
#include "opencv2/imgproc.hpp"

void cornerEigenValsVecs(
    const cv::Mat& src,
    cv::Mat& eigenv,
    int block_size,
    int aperture_size,
    int op_type,
    double k,
    int borderType,
    GFTTCornerDerivativeType derivType /* PCL code - extra argument compared to the original */
) {

    int depth = src.depth();
    double scale = (double)(1 << ((aperture_size > 0 ? aperture_size : 3) - 1)) * block_size;
    if (aperture_size < 0)
        scale *= 2.0;
    if (depth == CV_8U)
        scale *= 255.0;
    scale = 1.0/scale;

    CV_Assert( src.type() == CV_8UC1 || src.type() == CV_32FC1 );

    cv::Mat Dx, Dy;
    /* Original */
    // if( aperture_size > 0 ) {
    //     Sobel( src, Dx, CV_32F, 1, 0, aperture_size, scale, 0, borderType );
    //     Sobel( src, Dy, CV_32F, 0, 1, aperture_size, scale, 0, borderType );
    // }
    // else {
    //     Scharr( src, Dx, CV_32F, 1, 0, scale, 0, borderType );
    //     Scharr( src, Dy, CV_32F, 0, 1, scale, 0, borderType );
    // }

    /* PCL code - replacing the segment above */
    if (derivType == GFTTCornerDerivativeType::BINARIZED) {
        // Custom scale factor for binarized derivatives
        scale = 1.0 / (block_size * (depth == CV_8U ? 255.0 : 1.0));

        binarizedDerivative(src, Dx, true, scale);
        binarizedDerivative(src, Dy, false, scale);
    }
    else {
        if (aperture_size > 0) {
            cv::Sobel(src, Dx, CV_32F, 1, 0, aperture_size, scale, 0, borderType);
            cv::Sobel(src, Dy, CV_32F, 0, 1, aperture_size, scale, 0, borderType);
        }
        else {
            cv::Scharr(src, Dx, CV_32F, 1, 0, scale, 0, borderType);
            cv::Scharr(src, Dy, CV_32F, 0, 1, scale, 0, borderType);
        }
    }

    cv::Size size = src.size();
    cv::Mat cov(size, CV_32FC3);
    int i, j;

    for (i = 0; i < size.height; i++) {
        float* cov_data = cov.ptr<float>(i);
        const float* dxdata = Dx.ptr<float>(i);
        const float* dydata = Dy.ptr<float>(i);

        for(j = 0; j < size.width; j++ ) {
            float dx = dxdata[j];
            float dy = dydata[j];

            cov_data[j*3] = dx*dx;
            cov_data[j*3+1] = dx*dy;
            cov_data[j*3+2] = dy*dy;
        }
    }

    cv::boxFilter(
        cov,
        cov,
        cov.depth(),
        cv::Size(block_size, block_size),
        cv::Point(-1,-1),
        false,
        borderType
    );

    if (op_type == cv_internal::MINEIGENVAL) {
        cv_internal::calcMinEigenVal(cov, eigenv);
    }
    else if (op_type == cv_internal::HARRIS) {
        cv_internal::calcHarris(cov, eigenv, k);
    }
    else {
        throw std::invalid_argument("Invalid op_type in cornerEigenValsVecs() call!");
    }
}


void cornerMinEigenVal(
    cv::InputArray _src,
    cv::OutputArray _dst,
    int blockSize,
    int ksize,
    int borderType,
    GFTTCornerDerivativeType derivType
) {
    cv::Mat src = _src.getMat();
    _dst.create( src.size(), CV_32FC1 );
    cv::Mat dst = _dst.getMat();

    cornerEigenValsVecs(
        src,
        dst,
        blockSize,
        ksize,
        cv_internal::MINEIGENVAL,
        0,
        borderType,
        derivType
    );
}


void cornerHarris(
    cv::InputArray _src,
    cv::OutputArray _dst,
    int blockSize,
    int ksize,
    double k,
    int borderType,
    GFTTCornerDerivativeType derivType
) {

    cv::Mat src = _src.getMat();
    _dst.create(src.size(), CV_32FC1);
    cv::Mat dst = _dst.getMat();

    cornerEigenValsVecs(
        src,
        dst,
        blockSize,
        ksize,
        cv_internal::HARRIS,
        k,
        borderType,
        derivType
    );
}
