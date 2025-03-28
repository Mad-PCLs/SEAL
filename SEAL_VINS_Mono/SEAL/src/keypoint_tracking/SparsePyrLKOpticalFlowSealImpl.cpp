// License: See the LICENSE in the header file.

#include "keypoint_tracking/SparsePyrLKOpticalFlowSealImpl.hpp"

SparsePyrLKOpticalFlowSealImpl::SparsePyrLKOpticalFlowSealImpl(
    cv::Size winSize,
    int maxLevel,
    cv::TermCriteria criteria,
    int flags,
    double minEigThreshold,
    LKSpatialDerivativeType spatialDerivativeType,
    LKPyrDownFilterType pyrDownFilterType
)
: winSize(winSize), maxLevel(maxLevel), criteria(criteria), flags(flags),
    minEigThreshold(minEigThreshold), spatialDerivativeType(spatialDerivativeType),
    pyrDownFilterType(pyrDownFilterType)
{ }


// Default values from here: https://github.com/opencv/opencv/blob/4.x/modules/video/include/opencv2/video/tracking.hpp#L124
/* PCL comment: VINS-Mono compatibility change.
 * In this function cv::_OutputArray::DepthMask causes issues in OpenCV == 3.3.1.
 * An older implementation of cv::buildOpticalFlowPyramid is available so we use
 * it to patch this function, so it works with this older version of OpenCV.
 * Old(cv::buildOpticalFlowPyramid): https://github.com/opencv/opencv/blob/a871f9e4f7d83dc5851f965bdba5cd01bb7527fe/modules/video/src/lkpyramid.cpp#L700
 * New(cv::buildOpticalFlowPyramid): https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L747
 * 
 * The issue is with the type of the last argument when creating a cv::OutputArrayOfArrays,
 * which is essentially equivalent to (_OutputArray&)
 * Source: https://github.com/opencv/opencv/blob/ec5f7bb9f1d26fd2a78cf124a1c43f50b7de121f/modules/core/include/opencv2/core/mat.hpp#L445-L446
 * The difference in the type of the argument can be seen here:
 * Old(.create()): https://github.com/opencv/opencv/blob/a871f9e4f7d83dc5851f965bdba5cd01bb7527fe/modules/core/include/opencv2/core/mat.hpp#L350-L352
 * New(.create()): https://github.com/opencv/opencv/blob/ec5f7bb9f1d26fd2a78cf124a1c43f50b7de121f/modules/core/include/opencv2/core/mat.hpp#L368-L370
 */
int SparsePyrLKOpticalFlowSealImpl::buildOpticalFlowPyramid(
    cv::InputArray _img,
    cv::OutputArrayOfArrays pyramid,
    bool withDerivatives = true,
    int pyrBorder = cv::BORDER_REFLECT_101,
    int derivBorder = cv::BORDER_CONSTANT,
    bool tryReuseInputImage = true
) {
    cv::Mat img = _img.getMat();
    int pyrstep = withDerivatives ? 2 : 1;

    // Original:
    // pyramid.create(
    //     1,
    //     (maxLevel + 1) * pyrstep,
    //     0 /*type*/,
    //     -1,
    //     true,
    //     static_cast<cv::_OutputArray::DepthMask>(0)
    // );

    pyramid.create(
        1,
        (maxLevel + 1) * pyrstep,
        0 /*type*/,
        -1,
        true,
        0
    );

    int derivType = CV_MAKETYPE(cv::DataType<deriv_type>::depth, img.channels() * 2);

    // level 0
    bool lvl0IsSet = false;
    if (tryReuseInputImage && img.isSubmatrix()
          && (pyrBorder & cv::BORDER_ISOLATED) == 0)
    {
        cv::Size wholeSize;
        cv::Point ofs;
        img.locateROI(wholeSize, ofs);
        if (ofs.x >= winSize.width
              && ofs.y >= winSize.height
              && ofs.x + img.cols + winSize.width <= wholeSize.width
              && ofs.y + img.rows + winSize.height <= wholeSize.height)
        {
            pyramid.getMatRef(0) = img;
            lvl0IsSet = true;
        }
    }

    if (!lvl0IsSet) {
        cv::Mat& temp = pyramid.getMatRef(0);

        if (!temp.empty())
            temp.adjustROI(winSize.height, winSize.height, winSize.width, winSize.width);

        if (temp.type() != img.type()
              || temp.cols != winSize.width*2 + img.cols
              || temp.rows != winSize.height * 2 + img.rows)
            temp.create(img.rows + winSize.height*2, img.cols + winSize.width*2, img.type());

        if (pyrBorder == cv::BORDER_TRANSPARENT)
            img.copyTo(temp(cv::Rect(winSize.width, winSize.height, img.cols, img.rows)));
        else
            cv::copyMakeBorder(
                img,
                temp,
                winSize.height,
                winSize.height,
                winSize.width,
                winSize.width,
                pyrBorder
            );

        temp.adjustROI(-winSize.height, -winSize.height, -winSize.width, -winSize.width);
    }

    cv::Size sz = img.size();
    cv::Mat prevLevel = pyramid.getMatRef(0);
    cv::Mat thisLevel = prevLevel;

    for (int level = 0; level <= maxLevel; ++level) {
        if (level != 0) {
            cv::Mat& temp = pyramid.getMatRef(level * pyrstep);

            if (!temp.empty())
                temp.adjustROI(winSize.height, winSize.height, winSize.width, winSize.width);

            if (temp.type() != img.type()
                  || temp.cols != winSize.width*2 + sz.width
                  || temp.rows != winSize.height * 2 + sz.height)
                temp.create(sz.height + winSize.height*2, sz.width + winSize.width*2, img.type());

            thisLevel = temp(cv::Rect(winSize.width, winSize.height, sz.width, sz.height));

            pyrDown(
                prevLevel,
                thisLevel,
                sz,
                pyrBorder,
                pyrDownFilterType
            );

            if(pyrBorder != cv::BORDER_TRANSPARENT)
                copyMakeBorder(
                    thisLevel,
                    temp,
                    winSize.height,
                    winSize.height,
                    winSize.width,
                    winSize.width,
                    pyrBorder|cv::BORDER_ISOLATED
                );

            temp.adjustROI(-winSize.height, -winSize.height, -winSize.width, -winSize.width);
        }

        if (withDerivatives) {
            cv::Mat& deriv = pyramid.getMatRef(level * pyrstep + 1);

            if (!deriv.empty())
                deriv.adjustROI(winSize.height, winSize.height, winSize.width, winSize.width);
            if (deriv.type() != derivType
                  || deriv.cols != winSize.width*2 + sz.width
                  || deriv.rows != winSize.height * 2 + sz.height)
                deriv.create(sz.height + winSize.height*2, sz.width + winSize.width*2, derivType);

            cv::Mat derivI = deriv(cv::Rect(winSize.width, winSize.height, sz.width, sz.height));

            calcSpatialDerivative(thisLevel, derivI, spatialDerivativeType);

            if(derivBorder != cv::BORDER_TRANSPARENT)
                copyMakeBorder(
                    derivI,
                    deriv,
                    winSize.height,
                    winSize.height,
                    winSize.width,
                    winSize.width,
                    derivBorder|cv::BORDER_ISOLATED
                );

            deriv.adjustROI(-winSize.height, -winSize.height, -winSize.width, -winSize.width);
        }

        sz = cv::Size((sz.width+1)/2, (sz.height+1)/2);
        if (sz.width <= winSize.width || sz.height <= winSize.height) {
            // Original:
            // pyramid.create(
            //     1,
            //     (level + 1) * pyrstep,
            //     0, /*type*/
            //     -1,
            //     true,
            //     static_cast<cv::_OutputArray::DepthMask>(0)
            // );

            pyramid.create(
                1,
                (level + 1) * pyrstep,
                0, /*type*/
                -1,
                true,
                0
            );
            return level;
        }

        prevLevel = thisLevel;
    }

    return maxLevel;
}

void SparsePyrLKOpticalFlowSealImpl::calc(
    cv::InputArray _prevImg,
    cv::InputArray _nextImg,
    cv::InputArray _prevPts,
    cv::InputOutputArray _nextPts,
    cv::OutputArray _status,
    cv::OutputArray _err
) {
    cv::Mat prevPtsMat = _prevPts.getMat();
    const int derivDepth = cv::DataType<deriv_type>::depth;

    CV_Assert( maxLevel >= 0 && winSize.width > 2 && winSize.height > 2 );

    int level=0, i, npoints;

    CV_Assert( (npoints = prevPtsMat.checkVector(2, CV_32F, true)) >= 0 );

    if (npoints == 0) {
        _nextPts.release();
        _status.release();
        _err.release();
        return;
    }

    if (!(flags & cv::OPTFLOW_USE_INITIAL_FLOW))
        _nextPts.create(prevPtsMat.size(), prevPtsMat.type(), -1, true);

    cv::Mat nextPtsMat = _nextPts.getMat();
    CV_Assert( nextPtsMat.checkVector(2, CV_32F, true) == npoints );

    const cv::Point2f* prevPts = prevPtsMat.ptr<cv::Point2f>();
    cv::Point2f* nextPts = nextPtsMat.ptr<cv::Point2f>();

    _status.create((int)npoints, 1, CV_8U, -1, true);
    cv::Mat statusMat = _status.getMat(), errMat;
    CV_Assert( statusMat.isContinuous() );
    uchar* status = statusMat.ptr();
    float* err = 0;

    for (i = 0; i < npoints; i++)
        status[i] = true;

    if (_err.needed()) {
        _err.create((int)npoints, 1, CV_32F, -1, true);
        errMat = _err.getMat();
        err = errMat.ptr<float>();
    }

    std::vector<cv::Mat> prevPyr, nextPyr;
    int levels1 = -1;
    int lvlStep1 = 1;
    int levels2 = -1;
    int lvlStep2 = 1;

    if (_prevImg.kind() == cv::_InputArray::STD_VECTOR_MAT) {
        _prevImg.getMatVector(prevPyr);

        levels1 = int(prevPyr.size()) - 1;

        if (levels1 % 2 == 1
            && prevPyr[0].channels() * 2 == prevPyr[1].channels() 
            && prevPyr[1].depth() == derivDepth)
        {
            lvlStep1 = 2;
            levels1 /= 2;
        }

        // ensure that pyramid has reqired padding
        if (levels1 > 0) {
            cv::Size fullSize;
            cv::Point ofs;
            prevPyr[lvlStep1].locateROI(fullSize, ofs);
        }

        if (levels1 < maxLevel)
            maxLevel = levels1;
    }

    if (_nextImg.kind() == cv::_InputArray::STD_VECTOR_MAT) {
        _nextImg.getMatVector(nextPyr);

        levels2 = int(nextPyr.size()) - 1;

        if (levels2 % 2 == 1
            && nextPyr[0].channels() * 2 == nextPyr[1].channels()
            && nextPyr[1].depth() == derivDepth)
        {
            lvlStep2 = 2;
            levels2 /= 2;
        }

        // ensure that pyramid has reqired padding
        if (levels2 > 0) {
            cv::Size fullSize;
            cv::Point ofs;
            nextPyr[lvlStep2].locateROI(fullSize, ofs);
        }

        if (levels2 < maxLevel)
            maxLevel = levels2;
    }

    if (levels1 < 0)
        maxLevel = buildOpticalFlowPyramid(_prevImg, prevPyr, false);

    if (levels2 < 0)
        maxLevel = buildOpticalFlowPyramid(_nextImg, nextPyr, false);

    if ((criteria.type & cv::TermCriteria::COUNT) == 0)
        criteria.maxCount = 30;
    else
        criteria.maxCount = std::min(std::max(criteria.maxCount, 0), 100);

    if ((criteria.type & cv::TermCriteria::EPS) == 0)
        criteria.epsilon = 0.01;
    else
        criteria.epsilon = std::min(std::max(criteria.epsilon, 0.), 10.);
    criteria.epsilon *= criteria.epsilon;

    // dI/dx ~ Ix, dI/dy ~ Iy
    cv::Mat derivIBuf;
    if (lvlStep1 == 1)
        derivIBuf.create(
            prevPyr[0].rows + winSize.height*2,
            prevPyr[0].cols + winSize.width*2,
            CV_MAKETYPE(derivDepth,
            prevPyr[0].channels()*2)
        );

    for (level = maxLevel; level >= 0; level--) {
        cv::Mat derivI;
        if (lvlStep1 == 1) {
            cv::Size imgSize = prevPyr[level * lvlStep1].size();
            cv::Mat _derivI(
                imgSize.height + winSize.height*2,
                imgSize.width + winSize.width*2,
                derivIBuf.type(),
                derivIBuf.ptr()
            );
            derivI = _derivI(cv::Rect(winSize.width, winSize.height, imgSize.width, imgSize.height));

            /* Original */
            // cv::calcScharrDeriv(prevPyr[level * lvlStep1], derivI);

            /* PCL addition: Replaces the code above. */
            calcSpatialDerivative(
                prevPyr[level * lvlStep1],
                derivI,
                spatialDerivativeType
            );

            cv::copyMakeBorder(
                derivI,
                _derivI,
                winSize.height,
                winSize.height,
                winSize.width,
                winSize.width,
                cv::BORDER_CONSTANT|cv::BORDER_ISOLATED
            );
        }
        else
            derivI = prevPyr[level * lvlStep1 + 1];

        CV_Assert(prevPyr[level * lvlStep1].size() == nextPyr[level * lvlStep2].size());
        CV_Assert(prevPyr[level * lvlStep1].type() == nextPyr[level * lvlStep2].type());

        cv::parallel_for_(
            cv::Range(0, npoints),
            cv_internal::LKTrackerInvoker(
                prevPyr[level * lvlStep1],
                derivI,
                nextPyr[level * lvlStep2],
                prevPts,
                nextPts,
                status,
                err,
                winSize,
                criteria,
                level,
                maxLevel,
                flags,
                (float)minEigThreshold
            )
        );
    }
}

/***********************************************
 **************** Due diligence ****************
 * getters and setters declaired by base class *
 ***********************************************/
cv::Size SparsePyrLKOpticalFlowSealImpl::getWinSize() const {
    return winSize;
}

void SparsePyrLKOpticalFlowSealImpl::setWinSize(cv::Size winSize_) {
    winSize = winSize_;
}

int SparsePyrLKOpticalFlowSealImpl::getMaxLevel() const {
    return maxLevel;
}

void SparsePyrLKOpticalFlowSealImpl::setMaxLevel(int maxLevel_) {
    maxLevel = maxLevel_;
}

cv::TermCriteria SparsePyrLKOpticalFlowSealImpl::getTermCriteria() const {
    return criteria;
}

void SparsePyrLKOpticalFlowSealImpl::setTermCriteria(cv::TermCriteria& crit_) {
    criteria = crit_;
}

int SparsePyrLKOpticalFlowSealImpl::getFlags() const {
    return flags;
}

void SparsePyrLKOpticalFlowSealImpl::setFlags(int flags_) {
    flags = flags_;
}

double SparsePyrLKOpticalFlowSealImpl::getMinEigThreshold() const {
    return minEigThreshold;
}

void SparsePyrLKOpticalFlowSealImpl::setMinEigThreshold(double minEigThreshold_) {
    minEigThreshold = minEigThreshold_;
}
