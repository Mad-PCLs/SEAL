#include "LKTrackerInvoker.hpp"

using namespace cv_internal;

LKTrackerInvoker::LKTrackerInvoker(
    const cv::Mat& prevImg,
    const cv::Mat& prevDeriv,
    const cv::Mat& nextImg,
    const cv::Point2f* prevPts,
    cv::Point2f* nextPts,
    uchar* status,
    float* err,
    cv::Size winSize,
    cv::TermCriteria criteria,
    int level,
    int maxLevel,
    int flags,
    float minEigThreshold
) 
    : prevImg(&prevImg), prevDeriv(&prevDeriv), nextImg(&nextImg),
      prevPts(prevPts), nextPts(nextPts), status(status), err(err),
      winSize(winSize), criteria(criteria), level(level), maxLevel(maxLevel),
      flags(flags), minEigThreshold(minEigThreshold) { }


void LKTrackerInvoker::operator()(const cv::Range& range) const
{
    // CV_INSTRUMENT_REGION();

    const int W_BITS = 14, W_BITS1 = 14;
    const float FLT_SCALE = 1.f/(1 << 20);

    cv::Point2f halfWin((winSize.width-1)*0.5f, (winSize.height-1)*0.5f);
    const cv::Mat& I = *prevImg;
    const cv::Mat& J = *nextImg;
    const cv::Mat& derivI = *prevDeriv;

    cv::AutoBuffer<cv::Point2f> prevPtsScaledData(range.end - range.start);
    cv::Point2f* prevPtsScaled = prevPtsScaledData.data();

    int j, cn = I.channels(), cn2 = cn*2;
    cv::AutoBuffer<deriv_type> _buf(winSize.area()*(cn + cn2));
    int derivDepth = cv::DataType<deriv_type>::depth;

    cv::Mat IWinBuf(winSize, CV_MAKETYPE(derivDepth, cn), _buf.data());
    cv::Mat derivIWinBuf(winSize, CV_MAKETYPE(derivDepth, cn2), _buf.data() + winSize.area()*cn);

    for( int ptidx = range.start; ptidx < range.end; ptidx++ )
    {
        cv::Point2f prevPt = prevPts[ptidx]*(float)(1./(1 << level));
        cv::Point2f nextPt;
        if( level == maxLevel )
        {
            if( flags & cv::OPTFLOW_USE_INITIAL_FLOW )
                nextPt = nextPts[ptidx]*(float)(1./(1 << level));
            else
                nextPt = prevPt;
        }
        else
            nextPt = nextPts[ptidx]*2.f;
        nextPts[ptidx] = nextPt;
        prevPtsScaled[ptidx-range.start] = prevPt;
    }

    // Not implemented: https://github.com/opencv/opencv/blob/12d182bf9e2fa518aafad75dd5b77835c75674b4/modules/video/src/hal_replacement.hpp#L54-L70
    // CALL_HAL(LKOpticalFlowLevel, cv_hal_LKOpticalFlowLevel,
    //     I.data, I.step, (const short*)derivI.data, derivI.step, J.data, J.step,
    //     I.cols, I.rows, I.channels(),
    //     (float*)prevPtsScaled, (float*)(nextPts+range.start), range.end-range.start,
    //     (level == 0) ? status+range.start: nullptr,
    //     err != nullptr ? err+range.start: nullptr,
    //     winSize.width, winSize.height, criteria.maxCount, criteria.epsilon,
    //     (flags & OPTFLOW_LK_GET_MIN_EIGENVALS) != 0,
    //     (float)minEigThreshold
    // );

    for( int ptidx = range.start; ptidx < range.end; ptidx++ )
    {
        cv::Point2f prevPt = prevPtsScaled[ptidx-range.start];
        cv::Point2i iprevPt, inextPt;
        prevPt -= halfWin;
        iprevPt.x = cvFloor(prevPt.x);
        iprevPt.y = cvFloor(prevPt.y);

        if( iprevPt.x < -winSize.width || iprevPt.x >= derivI.cols ||
            iprevPt.y < -winSize.height || iprevPt.y >= derivI.rows )
        {
            if( level == 0 )
            {
                status[ptidx] = false;
                if( err )
                    err[ptidx] = 0;
            }
            continue;
        }

        float a = prevPt.x - iprevPt.x;
        float b = prevPt.y - iprevPt.y;
        int iw00 = cvRound((1.f - a)*(1.f - b)*(1 << W_BITS));
        int iw01 = cvRound(a*(1.f - b)*(1 << W_BITS));
        int iw10 = cvRound((1.f - a)*b*(1 << W_BITS));
        int iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;

        int dstep = (int)(derivI.step/derivI.elemSize1());
        int stepI = (int)(I.step/I.elemSize1());
        int stepJ = (int)(J.step/J.elemSize1());
        acctype iA11 = 0, iA12 = 0, iA22 = 0;
        float A11, A12, A22;

        // extract the patch from the first image, compute covariation matrix of derivatives
        int x, y;
        for( y = 0; y < winSize.height; y++ )
        {
            const uchar* src = I.ptr() + (y + iprevPt.y)*stepI + iprevPt.x*cn;
            const deriv_type* dsrc = derivI.ptr<deriv_type>() + (y + iprevPt.y)*dstep + iprevPt.x*cn2;

            deriv_type* Iptr = IWinBuf.ptr<deriv_type>(y);
            deriv_type* dIptr = derivIWinBuf.ptr<deriv_type>(y);

            for (x = 0; x < winSize.width*cn; x++, dsrc += 2, dIptr += 2)
            {
                int ival = CV_DESCALE(src[x]*iw00 + src[x+cn]*iw01 +
                                      src[x+stepI]*iw10 + src[x+stepI+cn]*iw11, W_BITS1-5);
                int ixval = CV_DESCALE(dsrc[0]*iw00 + dsrc[cn2]*iw01 +
                                       dsrc[dstep]*iw10 + dsrc[dstep+cn2]*iw11, W_BITS1);
                int iyval = CV_DESCALE(dsrc[1]*iw00 + dsrc[cn2+1]*iw01 + dsrc[dstep+1]*iw10 +
                                       dsrc[dstep+cn2+1]*iw11, W_BITS1);

                Iptr[x] = (short)ival;
                dIptr[0] = (short)ixval;
                dIptr[1] = (short)iyval;

                iA11 += (itemtype)(ixval*ixval);
                iA12 += (itemtype)(ixval*iyval);
                iA22 += (itemtype)(iyval*iyval);
            }
        }

        A11 = iA11*FLT_SCALE;
        A12 = iA12*FLT_SCALE;
        A22 = iA22*FLT_SCALE;

        float D = A11*A22 - A12*A12;
        float minEig = (A22 + A11 - std::sqrt((A11-A22)*(A11-A22) +
                        4.f*A12*A12))/(2*winSize.width*winSize.height);

        if( err && (flags & cv::OPTFLOW_LK_GET_MIN_EIGENVALS) != 0 )
            err[ptidx] = (float)minEig;

        if( minEig < minEigThreshold || D < FLT_EPSILON )
        {
            if(level == 0)
                status[ptidx] = false;
            continue;
        }

        D = 1.f/D;

        cv::Point2f nextPt = nextPts[ptidx] - halfWin;
        cv::Point2f prevDelta;

        for( j = 0; j < criteria.maxCount; j++ )
        {
            inextPt.x = cvFloor(nextPt.x);
            inextPt.y = cvFloor(nextPt.y);

            if( inextPt.x < -winSize.width || inextPt.x >= J.cols ||
               inextPt.y < -winSize.height || inextPt.y >= J.rows )
            {
                if( level == 0 )
                    status[ptidx] = false;
                break;
            }

            a = nextPt.x - inextPt.x;
            b = nextPt.y - inextPt.y;
            iw00 = cvRound((1.f - a)*(1.f - b)*(1 << W_BITS));
            iw01 = cvRound(a*(1.f - b)*(1 << W_BITS));
            iw10 = cvRound((1.f - a)*b*(1 << W_BITS));
            iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
            acctype ib1 = 0, ib2 = 0;
            float b1, b2;

            for( y = 0; y < winSize.height; y++ )
            {
                const uchar* Jptr = J.ptr() + (y + inextPt.y)*stepJ + inextPt.x*cn;
                const deriv_type* Iptr = IWinBuf.ptr<deriv_type>(y);
                const deriv_type* dIptr = derivIWinBuf.ptr<deriv_type>(y);

                for(x = 0; x < winSize.width*cn; x++, dIptr += 2 )
                {
                    int diff = CV_DESCALE(Jptr[x]*iw00 + Jptr[x+cn]*iw01 +
                                          Jptr[x+stepJ]*iw10 + Jptr[x+stepJ+cn]*iw11,
                                          W_BITS1-5) - Iptr[x];
                    ib1 += (itemtype)(diff*dIptr[0]);
                    ib2 += (itemtype)(diff*dIptr[1]);
                }
            }

            b1 = ib1*FLT_SCALE;
            b2 = ib2*FLT_SCALE;

            cv::Point2f delta( (float)((A12*b2 - A22*b1) * D),
                          (float)((A12*b1 - A11*b2) * D));
            //delta = -delta;

            nextPt += delta;
            nextPts[ptidx] = nextPt + halfWin;

            if( delta.ddot(delta) <= criteria.epsilon )
                break;

            if( j > 0 && std::abs(delta.x + prevDelta.x) < 0.01 &&
               std::abs(delta.y + prevDelta.y) < 0.01 )
            {
                nextPts[ptidx] -= delta*0.5f;
                break;
            }
            prevDelta = delta;
        }

        if( status[ptidx] && err && level == 0 && (flags & cv::OPTFLOW_LK_GET_MIN_EIGENVALS) == 0 )
        {
            cv::Point2f nextPoint = nextPts[ptidx] - halfWin;
            cv::Point inextPoint;

            inextPoint.x = cvFloor(nextPoint.x);
            inextPoint.y = cvFloor(nextPoint.y);

            if( inextPoint.x < -winSize.width || inextPoint.x >= J.cols ||
                inextPoint.y < -winSize.height || inextPoint.y >= J.rows )
            {
                status[ptidx] = false;
                continue;
            }

            float aa = nextPoint.x - inextPoint.x;
            float bb = nextPoint.y - inextPoint.y;
            iw00 = cvRound((1.f - aa)*(1.f - bb)*(1 << W_BITS));
            iw01 = cvRound(aa*(1.f - bb)*(1 << W_BITS));
            iw10 = cvRound((1.f - aa)*bb*(1 << W_BITS));
            iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
            float errval = 0.f;

            for( y = 0; y < winSize.height; y++ )
            {
                const uchar* Jptr = J.ptr() + (y + inextPoint.y)*stepJ + inextPoint.x*cn;
                const deriv_type* Iptr = IWinBuf.ptr<deriv_type>(y);

                for( x = 0; x < winSize.width*cn; x++ )
                {
                    int diff = CV_DESCALE(Jptr[x]*iw00 + Jptr[x+cn]*iw01 +
                                          Jptr[x+stepJ]*iw10 + Jptr[x+stepJ+cn]*iw11,
                                          W_BITS1-5) - Iptr[x];
                    errval += std::abs((float)diff);
                }
            }
            err[ptidx] = errval * 1.f/(32*winSize.width*cn*winSize.height);
        }
    }
}
