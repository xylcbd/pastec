#include <opencv2/core/core_c.h>
#include <opencv2/calib3d/calib3d.hpp>

#include "imagereranker.h"


cv::Mat ImageReranker::findHomography( InputArray _points1, InputArray _points2,
                                       int method, double ransacReprojThreshold,
                                       OutputArray _mask )
{
    Mat points1 = _points1.getMat(), points2 = _points2.getMat();
    int npoints = points1.checkVector(2);
    CV_Assert( npoints >= 0 && points2.checkVector(2) == npoints &&
               points1.type() == points2.type());

    Mat H(3, 3, CV_64F);
    CvMat _pt1 = points1, _pt2 = points2;
    CvMat matH = H, c_mask, *p_mask = 0;
    if( _mask.needed() )
    {
        _mask.create(npoints, 1, CV_8U, -1, true);
        p_mask = &(c_mask = _mask.getMat());
    }
    bool ok = cvFindHomography( &_pt1, &_pt2, &matH, method, ransacReprojThreshold, p_mask ) > 0;
    if( !ok )
        H = Scalar(0);
    return H;
}


int ImageReranker::cvFindHomography( const CvMat* objectPoints, const CvMat* imagePoints,
                                   CvMat* __H, int method, double ransacReprojThreshold,
                                   CvMat* mask )
{
    const double confidence = 0.995;
    const int maxIters = 400;
    const double defaultRANSACReprojThreshold = 3;
    bool result = false;
    Ptr<CvMat> m, M, tempMask;

    double H[9];
    CvMat matH = cvMat( 3, 3, CV_64FC1, H );
    int count;

    CV_Assert( CV_IS_MAT(imagePoints) && CV_IS_MAT(objectPoints) );

    count = MAX(imagePoints->cols, imagePoints->rows);
    CV_Assert( count >= 4 );
    if( ransacReprojThreshold <= 0 )
        ransacReprojThreshold = defaultRANSACReprojThreshold;

    m = cvCreateMat( 1, count, CV_64FC2 );
    cvConvertPointsHomogeneous( imagePoints, m );

    M = cvCreateMat( 1, count, CV_64FC2 );
    cvConvertPointsHomogeneous( objectPoints, M );

    if( mask )
    {
        CV_Assert( CV_IS_MASK_ARR(mask) && CV_IS_MAT_CONT(mask->type) &&
            (mask->rows == 1 || mask->cols == 1) &&
            mask->rows*mask->cols == count );
    }
    if( mask || count > 4 )
        tempMask = cvCreateMat( 1, count, CV_8U );
    if( !tempMask.empty() )
        cvSet( tempMask, cvScalarAll(1.) );

    CvHomographyEstimator estimator(4);
    if( count == 4 )
        method = 0;

    assert(method == CV_RANSAC);
    result = estimator.runRANSAC( M, m, &matH, tempMask, ransacReprojThreshold, confidence, maxIters);

    if( result && count > 4 )
    {
        icvCompressPoints( (CvPoint2D64f*)M->data.ptr, tempMask->data.ptr, 1, count );
        count = icvCompressPoints( (CvPoint2D64f*)m->data.ptr, tempMask->data.ptr, 1, count );
        M->cols = m->cols = count;
        if( method == CV_RANSAC )
            estimator.runKernel( M, m, &matH );
        estimator.refine( M, m, &matH, 10 );
    }

    if( result )
        cvConvert( &matH, __H );

    if( mask && tempMask )
    {
        if( CV_ARE_SIZES_EQ(mask, tempMask) )
           cvCopy( tempMask, mask );
        else
           cvTranspose( tempMask, mask );
    }

    return (int)result;
}


template<typename T> int icvCompressPoints( T* ptr, const uchar* mask, int mstep, int count )
{
    int i, j;
    for( i = j = 0; i < count; i++ )
        if( mask[i*mstep] )
        {
            if( i > j )
                ptr[j] = ptr[i];
            j++;
        }
    return j;
}

