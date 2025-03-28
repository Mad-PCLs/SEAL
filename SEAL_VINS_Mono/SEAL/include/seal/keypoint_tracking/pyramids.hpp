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
    https://github.com/opencv/opencv/blob/4.x/modules/imgproc/src/pyramids.cpp
PCL */

#pragma once

#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#include "opencv_internal/pyramids.hpp"

#include "types/LKPyrDownFilterTypes.hpp"
#include "keypoint_tracking/PyrDownInvoker.hpp"

typedef void (*PyrFunc)(const cv::Mat&, cv::Mat&, int, LKPyrDownFilterType);

template<class CastOp>
void pyrDown_(
    const cv::Mat& _src,
    cv::Mat& _dst,
    int borderType,
    LKPyrDownFilterType pyrDownFilterType
);

void pyrDown(
    cv::InputArray _src,
    cv::OutputArray _dst,
    const cv::Size& _dsz,
    int borderType,
    LKPyrDownFilterType pyrDownFilterType
);

void buildPyramid(
    cv::InputArray _src,
    cv::OutputArrayOfArrays _dst,
    int maxlevel,
    int borderType,
    LKPyrDownFilterType pyrDownFilterType
);
