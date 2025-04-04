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

/*************************************************************************************************************************
 * This file contains code copied from OpenCV's internal module.                                                         *
 * Source: https://github.com/opencv/opencv/blob/dbd3ef9a6f838d3ac42f8cf4d9d1dd5c709cb807/modules/imgproc/src/corner.cpp *
 *                                                                                                                       *
 * The purpose of copying this function is to use it without requiring                                                   *
 * modifications to OpenCV's source or recompilation.                                                                    *
 *************************************************************************************************************************/

#include <opencv2/opencv.hpp>

namespace cv_internal
{
    void calcMinEigenVal( const cv::Mat& _cov, cv::Mat& _dst );
    void calcHarris( const cv::Mat& _cov, cv::Mat& _dst, double k );

    enum { MINEIGENVAL=0, HARRIS=1 };
}
