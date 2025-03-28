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
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
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
    https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L849
PCL */

#pragma once

#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#include "types/LKSpatialDerivativeTypes.hpp"
#include "types/LKPyrDownFilterTypes.hpp"
#include "keypoint_tracking/gradients.hpp"
#include "../opencv_internal/include/LKTrackerInvoker.hpp"
#include "pyramids.hpp"

class SparsePyrLKOpticalFlowSealImpl : public cv::SparsePyrLKOpticalFlow
{
    public:
        SparsePyrLKOpticalFlowSealImpl(
            cv::Size winSize,
            int maxLevel,
            cv::TermCriteria criteria,
            int flags,
            double minEigThreshold,
            LKSpatialDerivativeType spatialDerivativeType,
            LKPyrDownFilterType pyrDownFilterType,
            bool useInitialFlow
        );

        int buildOpticalFlowPyramid(
            cv::InputArray _img,
            cv::OutputArrayOfArrays pyramid,
            bool withDerivatives,
            int pyrBorder,
            int derivBorder,
            bool tryReuseInputImage
        );

        void calc(
            cv::InputArray prevImg,
            cv::InputArray nextImg,
            cv::InputArray prevPts,
            cv::InputOutputArray nextPts,
            cv::OutputArray status,
            cv::OutputArray err = cv::noArray()
        );

        virtual cv::Size getWinSize() const override;
        virtual void setWinSize(cv::Size winSize_) override;

        virtual int getMaxLevel() const override;
        virtual void setMaxLevel(int maxLevel_) override;

        virtual cv::TermCriteria getTermCriteria() const override;
        virtual void setTermCriteria(cv::TermCriteria& crit_) override;

        virtual int getFlags() const override;
        virtual void setFlags(int flags_) override;

        virtual double getMinEigThreshold() const override;
        virtual void setMinEigThreshold(double minEigThreshold_) override;

private:
    cv::Size winSize;
    int maxLevel;
    cv::TermCriteria criteria;
    int flags;
    double minEigThreshold;
    LKSpatialDerivativeType spatialDerivativeType;
    LKPyrDownFilterType pyrDownFilterType;
    bool useInitialFlow;
};
