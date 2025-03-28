// License: See the LICENSE in the header file.

#include "ScharrDerivInvoker.hpp"

// Original header file: https://github.com/opencv/opencv/blob/12d182bf9e2fa518aafad75dd5b77835c75674b4/modules/video/src/lkpyramid.hpp#L10
cv_internal::ScharrDerivInvoker::ScharrDerivInvoker(
    const cv::Mat& _src,
    const cv::Mat& _dst
) : src(_src), dst(_dst) { }

// Original: https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L74
/* PCL comment: VINS-Mono compatibility change.
 * ScharrDerivInvoker was not found in the release branch of OpenCV version 3.3.1
 * (source: https://github.com/opencv/opencv/tree/a871f9e4f7d83dc5851f965bdba5cd01bb7527fe).
 * 
 * By comparing the old and new version of LKTrackerInvoker on github we can find what has
 * changed in the way the data in cv::AutoBuffer are accessed, hence we patch this manually.
 * Old(LKTrackerInvoker): https://github.com/opencv/opencv/blob/a871f9e4f7d83dc5851f965bdba5cd01bb7527fe/modules/video/src/lkpyramid.cpp#L194
 * New(LKTrackerInvoker): https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L206
 * Hence the difference is that we change .data() to a simple cast.
 */
void cv_internal::ScharrDerivInvoker::operator()(const cv::Range& range) const {
    int rows = src.rows, cols = src.cols, cn = src.channels(), colsn = cols*cn;

    int x, y, delta = (int)cv::alignSize((cols + 2)*cn, 16);
    cv::AutoBuffer<deriv_type> _tempBuf(delta*2 + 64);
    // Original: Before patching for OpenCV == 3.3.1
    // deriv_type *trow0 = cv::alignPtr(_tempBuf.data() + cn, 16), *trow1 = cv::alignPtr(trow0 + delta, 16);
    deriv_type *trow0 = cv::alignPtr((deriv_type*)_tempBuf + cn, 16), *trow1 = cv::alignPtr(trow0 + delta, 16);

    for( y = range.start; y < range.end; y++ )
    {
        const uchar* srow0 = src.ptr<uchar>(y > 0 ? y-1 : rows > 1 ? 1 : 0);
        const uchar* srow1 = src.ptr<uchar>(y);
        const uchar* srow2 = src.ptr<uchar>(y < rows-1 ? y+1 : rows > 1 ? rows-2 : 0);
        deriv_type* drow = (deriv_type *)dst.ptr<deriv_type>(y);

        // do vertical convolution
        x = 0;

        for( ; x < colsn; x++ )
        {
            int t0 = (srow0[x] + srow2[x])*3 + srow1[x]*10;
            int t1 = srow2[x] - srow0[x];
            trow0[x] = (deriv_type)t0;
            trow1[x] = (deriv_type)t1;
        }

        // make border
        int x0 = (cols > 1 ? 1 : 0)*cn, x1 = (cols > 1 ? cols-2 : 0)*cn;
        for( int k = 0; k < cn; k++ )
        {
            trow0[-cn + k] = trow0[x0 + k]; trow0[colsn + k] = trow0[x1 + k];
            trow1[-cn + k] = trow1[x0 + k]; trow1[colsn + k] = trow1[x1 + k];
        }

        // do horizontal convolution, interleave the results and store them to dst
        for(x = 0; x < colsn; x++ )
        {
            deriv_type t0 = (deriv_type)(trow0[x+cn] - trow0[x-cn]);
            deriv_type t1 = (deriv_type)((trow1[x+cn] + trow1[x-cn])*3 + trow1[x]*10);
            drow[x*2] = t0; drow[x*2+1] = t1;
        }
    }
}


// Original function: https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L59
void cv_internal::calcScharrDeriv(const cv::Mat& src, cv::Mat& dst) {
    int rows = src.rows, cols = src.cols, cn = src.channels();
    dst.create(rows, cols, CV_MAKETYPE(cv::DataType<deriv_type>::depth, cn*2));

    /* Original:
    *   We ignore HAL, since it is not implemented yet:
    *   https://github.com/opencv/opencv/blob/4.x/modules/video/src/hal_replacement.hpp#L84-L93
    */
    // CALL_HAL(ScharrDeriv, cv_hal_ScharrDeriv, src.data, src.step, (short*)dst.data, dst.step, cols, rows, cn);
    // cv::parallel_for_(cv::Range(0, rows), cv::ScharrDerivInvoker(src, dst), cv::getNumThreads());

    /* PCL: replaces the above original code */
    cv::parallel_for_(cv::Range(0, rows), ScharrDerivInvoker(src, dst), cv::getNumThreads());
}
