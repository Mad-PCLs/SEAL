// License: See the LICENSE in the header file.

#include "keypoint_tracking/pyramids.hpp"

/* Original function: https://github.com/opencv/opencv/blob/4.x/modules/imgproc/src/pyramids.cpp#L882 */
template<class CastOp>
void pyrDown_(
    const cv::Mat& _src,
    cv::Mat& _dst,
    int borderType,
    LKPyrDownFilterType pyrDownFilterType
) {
    const int PD_SZ = 5;
    CV_Assert( !_src.empty() );
    cv::Size ssize = _src.size(), dsize = _dst.size();
    int cn = _src.channels();

    cv::AutoBuffer<int> _tabM(dsize.width * cn);
    cv::AutoBuffer<int> _tabL(cn * (PD_SZ + 2));
    cv::AutoBuffer<int> _tabR(cn * (PD_SZ + 2));
    int *tabM = _tabM.data();
    int *tabL = _tabL.data();
    int *tabR = _tabR.data();

    CV_Assert(ssize.width > 0
                && ssize.height > 0
                && std::abs(dsize.width*2 - ssize.width) <= 2
                && std::abs(dsize.height*2 - ssize.height) <= 2);

    int width0 = std::min((ssize.width-PD_SZ/2 - 1)/2 + 1, dsize.width);

    for (int x = 0; x <= PD_SZ+1; x++) {
        int sx0 = cv::borderInterpolate(x - PD_SZ/2, ssize.width, borderType)*cn;
        int sx1 = cv::borderInterpolate(x + width0*2 - PD_SZ/2, ssize.width, borderType)*cn;

        for (int k = 0; k < cn; k++) {
            tabL[x*cn + k] = sx0 + k;
            tabR[x*cn + k] = sx1 + k;
        }
    }

    for (int x = 0; x < dsize.width*cn; x++)
        tabM[x] = (x/cn)*2*cn + x % cn;

    int *tabLPtr = tabL;
    int *tabRPtr = tabR;

    /* PCL addition: Apply custom filter if enabled */
    cv::Mat _src_new = _src.clone();
    bool disableGaussian = true;
    if (pyrDownFilterType == LKPyrDownFilterType::MEDIAN_3x3)
    {
        cv::medianBlur(_src, _src_new, 3);
    } else if (pyrDownFilterType == LKPyrDownFilterType::BOX_3x3) {
        cv::blur(_src, _src_new, cv::Size(3, 3));
    } else if (pyrDownFilterType == LKPyrDownFilterType::BOX_2x2) {
        cv::blur(_src, _src_new, cv::Size(2, 2));
    } else if (pyrDownFilterType == LKPyrDownFilterType::DIRECT_SUBSAMPLE) {
        // Nothing needs to be done
    } else if (pyrDownFilterType == LKPyrDownFilterType::GAUSSIAN_3x3) {
        cv::GaussianBlur(_src, _src_new, cv::Size(3, 3), 0, 0);
    } else if (pyrDownFilterType == LKPyrDownFilterType::GAUSSIAN_5x5) {
        // cv::GaussianBlur(_src, _src_new, cv::Size(5, 5), 0, 0);
        disableGaussian = false;
    } else {
        CV_Error(cv::Error::StsBadArg, "Invalid pyrDown filter type");
    }
    /* PCL addition: End, the rest is the same as in the original OpenCV code. */

    cv::parallel_for_(
        cv::Range(0, dsize.height),
        PyrDownInvoker<CastOp>(
            _src_new,
            _dst,
            borderType,
            &tabRPtr,
            &tabM,
            &tabLPtr,
            disableGaussian /* PCL addition: Extra argument */
        ),
        cv::getNumThreads()
    );
}

/* Original function: https://github.com/opencv/opencv/blob/4.x/modules/imgproc/src/pyramids.cpp#L1267 */
// Same as the original function, but instead of calling cv::pyrDown_ internally,
// it calls the updated pyrDown_ function that supports different types of pyrDownFilterType.
void pyrDown(
    cv::InputArray _src,
    cv::OutputArray _dst,
    const cv::Size& _dsz,
    int borderType,
    LKPyrDownFilterType pyrDownFilterType
) {
    CV_Assert(borderType != cv::BORDER_CONSTANT);

    cv::Mat src = _src.getMat();
    cv::Size dsz = _dsz.empty() ? cv::Size((src.cols + 1)/2, (src.rows + 1)/2) : _dsz;
    _dst.create( dsz, src.type() );
    cv::Mat dst = _dst.getMat();
    int depth = src.depth();

    PyrFunc func = 0;
    if( depth == CV_8U )
        func = pyrDown_< cv_internal::FixPtCast<uchar, 8> >;
    else if( depth == CV_16S )
        func = pyrDown_< cv_internal::FixPtCast<short, 8> >;
    else if( depth == CV_16U )
        func = pyrDown_< cv_internal::FixPtCast<ushort, 8> >;
    else if( depth == CV_32F )
        func = pyrDown_< cv_internal::FltCast<float, 8> >;
    else if( depth == CV_64F )
        func = pyrDown_< cv_internal::FltCast<double, 8> >;
    else
        CV_Error( cv::Error::StsUnsupportedFormat, "" );

    func( src, dst, borderType, pyrDownFilterType );
}


/* Original function: https://github.com/opencv/opencv/blob/4.x/modules/imgproc/src/pyramids.cpp#L1534 */
// Same as the original function, but instead of calling cv::pyrDown internally,
// it calls the updated pyrDown function that supports different types of pyrDownFilterType.
void buildPyramid(
    cv::InputArray _src,
    cv::OutputArrayOfArrays _dst,
    int maxlevel,
    int borderType,
    LKPyrDownFilterType pyrDownFilterType
) {
    CV_Assert(borderType != cv::BORDER_CONSTANT);

    if (_src.dims() <= 2 && _dst.isUMatVector()) {
        cv::UMat src = _src.getUMat();
        _dst.create( maxlevel + 1, 1, 0 );
        _dst.getUMatRef(0) = src;
        for( int i = 1; i <= maxlevel; i++ )
            pyrDown(
                _dst.getUMatRef(i-1),
                _dst.getUMatRef(i),
                cv::Size(),
                borderType,
                pyrDownFilterType
            );
        return;
    }

    cv::Mat src = _src.getMat();
    _dst.create( maxlevel + 1, 1, 0 );
    _dst.getMatRef(0) = src;

    for (int i=1; i <= maxlevel; i++)
        pyrDown(
            _dst.getMatRef(i-1),
            _dst.getMatRef(i),
            cv::Size(),
            borderType,
            pyrDownFilterType
        );
}
