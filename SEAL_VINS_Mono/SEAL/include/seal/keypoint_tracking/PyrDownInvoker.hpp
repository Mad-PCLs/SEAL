/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2014-2015, Itseez Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/* PCL
    Custom LK Implementation Based on:
    https://github.com/opencv/opencv/blob/4.x/modules/imgproc/src/pyramids.cpp#L859
PCL */

#pragma once

#include <opencv2/opencv.hpp>

#include "types/LKPyrDownFilterTypes.hpp"
#include "opencv_internal/pyramids.hpp"

template<class CastOp>
struct PyrDownInvoker : cv::ParallelLoopBody {
    PyrDownInvoker(
        const cv::Mat& src, 
        const cv::Mat& dst, 
        int borderType, 
        int** tabR, 
        int** tabM, 
        int** tabL, 
        bool disableGaussian /* PCL addition: Extra parameter */
    ) 
    : _src(&src), _dst(&dst), _borderType(borderType), _tabR(tabR), _tabM(tabM), _tabL(tabL), 
      disableGaussian(disableGaussian) {}

    void operator()(const cv::Range& range) const override;

    const cv::Mat *_src;
    const cv::Mat *_dst;
    int _borderType;
    int** _tabR;
    int** _tabM;
    int** _tabL;
    
    /* PCL addition: Extra parameter */
    bool disableGaussian;
};

/* PCL comment: VINS-Mono compatibility change.
 * In the release branch of OpenCV version 3.3.1, the entire pyramids.cpp file is missing and
 * as a result PyrDownInvoker as well.
 * (source: https://github.com/opencv/opencv/tree/a871f9e4f7d83dc5851f965bdba5cd01bb7527fe).
 * 
 * By comparing the old and new version of LKTrackerInvoker on github we can find what has
 * changed in the way the data in cv::AutoBuffer are accessed, hence we patch this manually.
 * Old(LKTrackerInvoker): https://github.com/opencv/opencv/blob/a871f9e4f7d83dc5851f965bdba5cd01bb7527fe/modules/video/src/lkpyramid.cpp#L194
 * New(LKTrackerInvoker): https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L206
 * Hence the difference is that we change .data() to a simple cast.
 */
template<class CastOp>
void PyrDownInvoker<CastOp>::operator()(const cv::Range& range) const
{
    const int PD_SZ = 5;
    typedef typename CastOp::type1 WT;
    typedef typename CastOp::rtype T;
    cv::Size ssize = _src->size(), dsize = _dst->size();
    int cn = _src->channels();
    int bufstep = (int)cv::alignSize(dsize.width*cn, 16);
    cv::AutoBuffer<WT> _buf(bufstep*PD_SZ + 16);
    WT* buf = cv::alignPtr((WT*)_buf, 16);
    WT* rows[PD_SZ];
    CastOp castOp;

    int sy0 = -PD_SZ/2, sy = range.start * 2 + sy0, width0 = std::min((ssize.width-PD_SZ/2-1)/2 + 1, dsize.width);

    ssize.width *= cn;
    dsize.width *= cn;
    width0 *= cn;

    for (int y = range.start; y < range.end; y++) {
        T* dst = (T*)_dst->ptr<T>(y);

        /* PCL addition: If statement below does not exist in the original OpenCV code. */
        // If Gaussian is disabled, directly subsample every second pixel
        if (disableGaussian) {
            // Corresponding source row in the original image (taking every second row)
            int sy = y * 2;
            const T* src = _src->ptr<T>(sy);

            for (int x = 0; x < dsize.width; x++) {
                // Directly subsample: Take every second pixel in each row
                int sx = x * 2 * cn; // Corresponding pixel in the original image
                for (int c = 0; c < cn; c++) {
                    dst[x * cn + c] = src[sx + c];
                }
            }

            // Skip the Gaussian convolution part
            continue;
        }

        // Otherwise, perform Gaussian filtering
        /* PCL addition: At this point PCL addition ENDS. The rest comes directly from the OpenCV repo. */

        WT *row0, *row1, *row2, *row3, *row4;

        // fill the ring buffer (horizontal convolution and decimation)
        int sy_limit = y*2 + 2;
        for( ; sy <= sy_limit; sy++ )
        {
            WT* row = buf + ((sy - sy0) % PD_SZ)*bufstep;
            int _sy = cv::borderInterpolate(sy, ssize.height, _borderType);
            const T* src = _src->ptr<T>(_sy);

            do {
                int x = 0;
                const int* tabL = *_tabL;
                for( ; x < cn; x++ )
                {
                    row[x] = src[tabL[x+cn*2]]*6 + (src[tabL[x+cn]] + src[tabL[x+cn*3]])*4 +
                        src[tabL[x]] + src[tabL[x+cn*4]];
                }

                if( x == dsize.width )
                    break;

                if( cn == 1 )
                {
                    x += cv_internal::PyrDownVecH<T, WT, 1>(src + x * 2 - 2, row + x, width0 - x);
                    for( ; x < width0; x++ )
                        row[x] = src[x*2]*6 + (src[x*2 - 1] + src[x*2 + 1])*4 +
                            src[x*2 - 2] + src[x*2 + 2];
                }
                else if( cn == 2 )
                {
                    x += cv_internal::PyrDownVecH<T, WT, 2>(src + x * 2 - 4, row + x, width0 - x);
                    for( ; x < width0; x += 2 )
                    {
                        const T* s = src + x*2;
                        WT t0 = s[0] * 6 + (s[-2] + s[2]) * 4 + s[-4] + s[4];
                        WT t1 = s[1] * 6 + (s[-1] + s[3]) * 4 + s[-3] + s[5];
                        row[x] = t0; row[x + 1] = t1;
                    }
                }
                else if( cn == 3 )
                {
                    x += cv_internal::PyrDownVecH<T, WT, 3>(src + x * 2 - 6, row + x, width0 - x);
                    for( ; x < width0; x += 3 )
                    {
                        const T* s = src + x*2;
                        WT t0 = s[0]*6 + (s[-3] + s[3])*4 + s[-6] + s[6];
                        WT t1 = s[1]*6 + (s[-2] + s[4])*4 + s[-5] + s[7];
                        WT t2 = s[2]*6 + (s[-1] + s[5])*4 + s[-4] + s[8];
                        row[x] = t0; row[x+1] = t1; row[x+2] = t2;
                    }
                }
                else if( cn == 4 )
                {
                    x += cv_internal::PyrDownVecH<T, WT, 4>(src + x * 2 - 8, row + x, width0 - x);
                    for( ; x < width0; x += 4 )
                    {
                        const T* s = src + x*2;
                        WT t0 = s[0]*6 + (s[-4] + s[4])*4 + s[-8] + s[8];
                        WT t1 = s[1]*6 + (s[-3] + s[5])*4 + s[-7] + s[9];
                        row[x] = t0; row[x+1] = t1;
                        t0 = s[2]*6 + (s[-2] + s[6])*4 + s[-6] + s[10];
                        t1 = s[3]*6 + (s[-1] + s[7])*4 + s[-5] + s[11];
                        row[x+2] = t0; row[x+3] = t1;
                    }
                }
                else
                {
                    for( ; x < width0; x++ )
                    {
                        int sx = (*_tabM)[x];
                        row[x] = src[sx]*6 + (src[sx - cn] + src[sx + cn])*4 +
                            src[sx - cn*2] + src[sx + cn*2];
                    }
                }

                // tabR
                const int* tabR = *_tabR;
                for (int x_ = 0; x < dsize.width; x++, x_++)
                {
                    row[x] = src[tabR[x_+cn*2]]*6 + (src[tabR[x_+cn]] + src[tabR[x_+cn*3]])*4 +
                        src[tabR[x_]] + src[tabR[x_+cn*4]];
                }
            } while (0);
        }

        // do vertical convolution and decimation and write the result to the destination image
        for (int k = 0; k < PD_SZ; k++)
            rows[k] = buf + ((y*2 - PD_SZ/2 + k - sy0) % PD_SZ)*bufstep;
        row0 = rows[0]; row1 = rows[1]; row2 = rows[2]; row3 = rows[3]; row4 = rows[4];

        int x = cv_internal::PyrDownVecV<WT, T>(rows, dst, dsize.width);
        for (; x < dsize.width; x++ )
            dst[x] = castOp(row2[x]*6 + (row1[x] + row3[x])*4 + row0[x] + row4[x]);
    }
}
