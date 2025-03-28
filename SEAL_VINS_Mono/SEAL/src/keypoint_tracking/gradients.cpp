// License: See the LICENSE in the header file.

#include "keypoint_tracking/gradients.hpp"

// Calculates the derivatives using a Sobel filter.
void calcSobelDeriv(const cv::Mat& src, cv::Mat& dst) {
    // Depth of the output image
    int ddepth = CV_64F;

    // Sobel filters for x and y direction
    cv::Mat sobelX, sobelY;
    // Apply Sobel filter in x direction
    cv::Sobel(src, sobelX, ddepth, 1, 0, 3);
    // Apply Sobel filter in y direction
    cv::Sobel(src, sobelY, ddepth, 0, 1, 3);

    // TODO: PCL: Can use integer scale factor. For now, using float for flexibility
    //            If converting to int scale factor, we can replace ddepth
    //            with CV_16S and remove the .converTo() conversions.
    float scaleFactor = 4;

    // Scale the gradients
    sobelX *= scaleFactor;
    sobelY *= scaleFactor;

    sobelX.convertTo(sobelX, CV_16S);
    sobelY.convertTo(sobelY, CV_16S);

    // Merge the results into a single matrix
    std::vector<cv::Mat> sobelChannels = {sobelX, sobelY};
    cv::merge(sobelChannels, dst);
}

// Custom implementation of an estimate for the derivative
// of a binarized image, as implemented in SEAL.
void calcBinarizedDeriv(const cv::Mat& src, cv::Mat& dst) {
    // Depth of the output image
    int ddepth = CV_16S;

    // Define binarized Sobel-like kernels (1x3 and 3x1) using cv::Mat
    cv::Mat kernelX = (cv::Mat_<int>(1, 3) << -1, 0, 1);
    cv::Mat kernelY = (cv::Mat_<int>(3, 1) << -1, 0, 1);

    // Apply the custom binarized kernels to compute derivatives
    cv::Mat binarizedX, binarizedY;
    cv::filter2D(src, binarizedX, ddepth, kernelX);
    cv::filter2D(src, binarizedY, ddepth, kernelY);

    // Apply scale factor
    int scaleFactor = 16;

    binarizedX *= scaleFactor;
    binarizedY *= scaleFactor;

    // Merge the results into a single matrix
    std::vector<cv::Mat> binarizedChannels = {binarizedX, binarizedY};
    cv::merge(binarizedChannels, dst);
}

// Essentially a verison of calcScharrDeriv that uses a different scale.
// The function is heavily based on OpenCV implementation, with SIMD removed
// and 2 constants updated.
void calcScharrScaledDeriv(const cv::Mat& src, cv::Mat& dst) {
    /* Original source: https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L59-L63 */
    int rows = src.rows, cols = src.cols, cn = src.channels();
    int colsn = cols*cn;
    dst.create(rows, cols, CV_MAKETYPE(cv::DataType<deriv_type>::depth, cn*2));

    /* Original source: https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L79 */
    int x, y, delta = (int)cv::alignSize((cols + 2)*cn, 16);
    cv::AutoBuffer<deriv_type> _tempBuf(delta*2 + 64);
    deriv_type *trow0 = cv::alignPtr(_tempBuf + cn, 16), *trow1 = cv::alignPtr(trow0 + delta, 16);

    for (y = 0; y < rows; y++) {
        const uchar* srow0 = src.ptr<uchar>(y > 0 ? y-1 : rows > 1 ? 1 : 0);
        const uchar* srow1 = src.ptr<uchar>(y);
        const uchar* srow2 = src.ptr<uchar>(y < rows-1 ? y+1 : rows > 1 ? rows-2 : 0);
        deriv_type* drow = dst.ptr<deriv_type>(y);

        // do vertical convolution
        for(x = 0; x < colsn; x++ ) {
            int t0 = (srow0[x] + srow2[x])*6 + srow1[x]*20;
            int t1 = srow2[x] - srow0[x];
            trow0[x] = (deriv_type)t0;
            trow1[x] = (deriv_type)t1;
        }

        // make border
        int x0 = (cols > 1 ? 1 : 0)*cn, x1 = (cols > 1 ? cols-2 : 0)*cn;
        for (int k = 0; k < cn; k++) {
            trow0[-cn + k] = trow0[x0 + k]; trow0[colsn + k] = trow0[x1 + k];
            trow1[-cn + k] = trow1[x0 + k]; trow1[colsn + k] = trow1[x1 + k];
        }

        // do horizontal convolution, interleave the results and store them to dst
        for (x = 0; x < colsn; x++) {
            deriv_type t0 = (deriv_type)(trow0[x+cn] - trow0[x-cn]);
            deriv_type t1 = (deriv_type)((trow1[x+cn] + trow1[x-cn])*6 + trow1[x]*20);
            drow[x*2] = t0; drow[x*2+1] = t1;
        }
    }
}

void calcSpatialDerivative(
    const cv::Mat& src,
    cv::Mat& dst,
    LKSpatialDerivativeType deriv_type
) {
    if (deriv_type == LKSpatialDerivativeType::BINARIZED)
        calcBinarizedDeriv(src, dst);
    else if (deriv_type == LKSpatialDerivativeType::SCHARR)
        cv_internal::calcScharrDeriv(src, dst);
    else if (deriv_type == LKSpatialDerivativeType::SCHARR_SCALED)
        calcScharrScaledDeriv(src, dst);
    else if (deriv_type == LKSpatialDerivativeType::SOBEL)
        calcSobelDeriv(src, dst);
    else
        throw std::invalid_argument("Invalid deriv_type in calcDeriv() call!");
}
