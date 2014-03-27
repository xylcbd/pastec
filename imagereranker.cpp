#include <iostream>
#include <cassert>
#include <math.h>

#include <algorithm>
#include <set>

#include <opencv2/calib3d/calib3d.hpp>

#include "imagereranker.h"


void ImageReranker::rerank(map<u_int32_t, list<Hit> > &imagesReqHits,
                           map<u_int32_t, vector<Hit> > &indexHits,
                           priority_queue<SearchResult> &rankedResultsIn,
                           priority_queue<SearchResult> &rankedResultsOut,
                           unsigned i_nbResults)
{
    set<u_int32_t> firstImageIds;

    // Extract the first i_nbResults ranked images.
    getFirstImageIds(rankedResultsIn, i_nbResults, firstImageIds);

    // Compute the histograms.
    map<u_int32_t, Histogram> histograms; // key: the image id, value: the corresponding histogram.

    for (map<u_int32_t, list<Hit> >::const_iterator it = imagesReqHits.begin();
         it != imagesReqHits.end(); ++it)
    {
        // Try to match all the visual words of the request image.
        const unsigned i_wordId = it->first;
        const list<Hit> &hits = it->second;

        for (list<Hit>::const_iterator it2 = hits.begin();
             it2 != hits.end(); ++it2)
        {
            // If there is several hits for the same word in the image...
            const u_int16_t i_angle1 = it2->i_angle;
            const vector<Hit> &hitIndex = indexHits[i_wordId];

            // Record the visual words that have already been matched.
            set<pair<u_int32_t, u_int32_t> > entriesRecorded;

            for (unsigned i = 0; i < hitIndex.size(); ++i)
            {
                const u_int32_t i_imageId = hitIndex[i].i_imageId;
                // Test if the image belongs to the image to rerank.
                if (firstImageIds.find(i_imageId) != firstImageIds.end())
                {
                    const u_int16_t i_angle2 = hitIndex[i].i_angle;
                    float f_diff = angleDiff(i_angle1, i_angle2);
                    unsigned bin = f_diff / 360 * HISTOGRAM_NB_BINS;
                    assert(bin < HISTOGRAM_NB_BINS);

                    pair<unsigned, unsigned> entry(i_imageId, bin);
                    /* Add the bin to the histogram only if it has not been
                     * already added for the given image. */
                    if (entriesRecorded.find(entry) == entriesRecorded.end())
                    {
                        histograms[i_imageId].bins[bin]++;
                        entriesRecorded.insert(entry);
                    }
                }
            }
        }
    }

    // Rank the images according to their histogram.
    for (map<unsigned, Histogram>::const_iterator it = histograms.begin();
         it != histograms.end(); ++it)
    {
        const unsigned i_imageId = it->first;
        const Histogram &histogram = it->second;
        unsigned i_maxVal = *max_element(histogram.bins, histogram.bins + HISTOGRAM_NB_BINS);
        rankedResultsOut.push(SearchResult(i_maxVal, i_imageId));
    }
}


void ImageReranker::rerankRANSAC(map<u_int32_t, list<Hit> > &imagesReqHits,
                                 map<u_int32_t, vector<Hit> > &indexHits,
                                 priority_queue<SearchResult> &rankedResultsIn,
                                 priority_queue<SearchResult> &rankedResultsOut,
                                 unsigned i_nbResults)
{
    set<u_int32_t> firstImageIds;

    map<u_int32_t, RANSACTask> imgTasks;

    // Extract the first i_nbResults ranked images.
    getFirstImageIds(rankedResultsIn, i_nbResults, firstImageIds);

    for (map<u_int32_t, list<Hit> >::const_iterator it = imagesReqHits.begin();
         it != imagesReqHits.end(); ++it)
    {
        // Try to match all the visual words of the request image.
        const unsigned i_wordId = it->first;
        const list<Hit> &hits = it->second;

        /* If there are several hits for the same visual word,
         we don't take it into account. */
        if (hits.size() > 1)
            continue;

        const Point2f point1(hits.front().x, hits.front().y);
        const vector<Hit> &hitIndex = indexHits[i_wordId];

        // Record the visual words that have already been matched.
        set<u_int32_t> entriesRecorded;

        for (unsigned i = 0; i < hitIndex.size(); ++i)
        {
            const u_int32_t i_imageId = hitIndex[i].i_imageId;
            // Test if the image belongs to the image to rerank.
            if (firstImageIds.find(i_imageId) != firstImageIds.end())
            {
                // Match only one keypoint with id i_wordId.
                if (entriesRecorded.find(i_imageId) == entriesRecorded.end())
                {
                    const Point2f point2(hitIndex[i].x, hitIndex[i].y);
                    imgTasks[i_imageId].points1.push_back(point1);
                    imgTasks[i_imageId].points2.push_back(point2);
                    entriesRecorded.insert(i_imageId);
                }
            }
        }
    }

    /* Find the homography with RANSAC.
     * Rank the images according to the number of matches. */
    map<u_int32_t, RANSACTask>::const_iterator it = imgTasks.begin();
    map<u_int32_t, RANSACTask>::const_iterator lastIt = imgTasks.end();

    pthread_mutex_t itMutex;
    pthread_mutex_t resultsMutex;

    pthread_mutex_init(&itMutex, NULL);
    pthread_mutex_init(&resultsMutex, NULL);

#define NB_THREADS 4

    RANSACThread *threads[NB_THREADS];
    for (unsigned i = 0; i < NB_THREADS; ++i)
    {
        threads[i] = new RANSACThread(&itMutex, it, lastIt,
                                      &resultsMutex, rankedResultsOut);
        threads[i]->start();
    }

    for (unsigned i = 0; i < NB_THREADS; ++i)
    {
        threads[i]->join();
        delete threads[i];
    }

    pthread_mutex_destroy(&itMutex);
    pthread_mutex_destroy(&resultsMutex);
}


void *RANSACThread::run()
{
    while (1)
    {
        pthread_mutex_lock(p_itMutex);
        if (mIt == mLastIt)
        {
            pthread_mutex_unlock(p_itMutex);
            break;
        }

        const u_int32_t i_imageId = mIt->first;
        const RANSACTask &task = mIt->second;
        mIt++; // Increment the iterator for the next iteration.
        pthread_mutex_unlock(p_itMutex);

        #define MIN_NB_INLINERS 8

        assert(task.points1.size() == task.points2.size());
        if (task.points1.size() >= MIN_NB_INLINERS)
        {
            Mat mask;
            findHomography(task.points1, task.points2, CV_RANSAC, 800, mask);

            // Count the number of inliners.
            unsigned i_nbInliners = 0;
            for (unsigned i = 0; i < task.points1.size(); ++i)
                if (mask.at<uchar>(0, i) == 1)
                    i_nbInliners++;

            if (i_nbInliners >= MIN_NB_INLINERS)
            {
                pthread_mutex_lock(p_resultsMutex);
                rankedResultsOut.push(SearchResult(i_nbInliners, i_imageId));
                pthread_mutex_unlock(p_resultsMutex);
            }
        }
    }

    return NULL;
}


void ImageReranker::rerankNearestNeighbors(map<u_int32_t, list<Hit> > &imagesReqHits,
                                           map<u_int32_t, vector<Hit> > &indexHits,
                                           priority_queue<SearchResult> &rankedResultsIn,
                                           priority_queue<SearchResult> &rankedResultsOut,
                                           unsigned i_nbResults)
{
    set<u_int32_t> firstImageIds;

    map<u_int32_t, PointList> imgMatchedKeypointsPos;
    map<u_int32_t, PointList> reqImgMatchedKeypointsPos;

    // Extract the first i_nbResults ranked images.
    getFirstImageIds(rankedResultsIn, i_nbResults, firstImageIds);

    for (map<u_int32_t, list<Hit> >::const_iterator it = imagesReqHits.begin();
         it != imagesReqHits.end(); ++it)
    {
        // Try to match all the visual words of the request image.
        const unsigned i_wordId = it->first;
        const list<Hit> &hits = it->second;

        for (list<Hit>::const_iterator it2 = hits.begin();
             it2 != hits.end(); ++it2)
        {
            // If there is several hits for the same word in the image...
            Mat point1(1, 2, CV_32S);
            point1.at<int32_t>(0, 0) = it2->x;
            point1.at<int32_t>(0, 1) = it2->y;
            const vector<Hit> &hitIndex = indexHits[i_wordId];

            for (unsigned i = 0; i < hitIndex.size(); ++i)
            {
                const u_int32_t i_imageId = hitIndex[i].i_imageId;
                // Test if the image belongs to the image to rerank.
                if (firstImageIds.find(i_imageId) != firstImageIds.end())
                {
                    Mat point2(1, 2, CV_32S);
                    point2.at<int32_t>(0, 0) = hitIndex[i].x;
                    point2.at<int32_t>(0, 1) = hitIndex[i].y;
                    reqImgMatchedKeypointsPos[i_imageId].push_back(point1);
                    imgMatchedKeypointsPos[i_imageId].push_back(point2);
                }
            }
        }
    }


    for (set<u_int32_t>::iterator it = firstImageIds.begin();
         it != firstImageIds.end(); ++it)
    {
        const u_int32_t imageId = *it;

        cout << "Reranking image " << imageId << endl;

        flann::Index indexImg(imgMatchedKeypointsPos[imageId], flann::KDTreeIndexParams());
        flann::Index indexReqImg(reqImgMatchedKeypointsPos[imageId], flann::KDTreeIndexParams());

        unsigned i_score = 0;
        for (int i = 0; i < imgMatchedKeypointsPos[imageId].rows; ++i)
        {
            #define NB_NEAREST_NEIGHBORS 5

            vector<int> indicesImg(NB_NEAREST_NEIGHBORS);
            vector<float> distsImg(NB_NEAREST_NEIGHBORS);
            indexImg.knnSearch(imgMatchedKeypointsPos[imageId].row(i), indicesImg, distsImg, NB_NEAREST_NEIGHBORS);

            vector<int> indicesImgReq(NB_NEAREST_NEIGHBORS);
            vector<float> distsImgReq(NB_NEAREST_NEIGHBORS);
            indexReqImg.knnSearch(reqImgMatchedKeypointsPos[imageId].row(i), indicesImgReq, distsImgReq, NB_NEAREST_NEIGHBORS);

            for (unsigned j = 0; j < NB_NEAREST_NEIGHBORS; ++j)
                if (find(indicesImgReq.begin(), indicesImgReq.end(),
                         indicesImg[j]) != indicesImgReq.end())
                    i_score++;
        }

        rankedResultsOut.push(SearchResult(imageId, i_score));
    }
}


/**
 * @brief Return the first ids of ranked images.
 * @param rankedResultsIn the ranked images.
 * @param i_nbResults the number of images to return.
 * @param firstImageIds a set to return the image ids.
 */
void ImageReranker::getFirstImageIds(priority_queue<SearchResult> &rankedResultsIn,
                                     unsigned i_nbResults, set<u_int32_t> &firstImageIds)
{
    unsigned i_res = 0;
    while(!rankedResultsIn.empty()
          && i_res < i_nbResults)
    {
        const SearchResult &res = rankedResultsIn.top();
        firstImageIds.insert(res.i_imageId);
        rankedResultsIn.pop();
        i_res++;
    }
}


float ImageReranker::angleDiff(unsigned i_angle1, unsigned i_angle2)
{
    // Convert the angle in the [-180, 180] range.
    float i1 = (float)i_angle1 * 360 / (1 << 16);
    float i2 = (float)i_angle2 * 360 / (1 << 16);

    i1 = i1 <= 180 ? i1 : i1 - 360;
    i2 = i2 <= 180 ? i2 : i2 - 360;

    // Compute the difference between the two angles.
    float diff = i1 - i2;
    if (diff < 0)
        diff += 360;

    assert(diff >= 0);
    assert(diff < 360);

    return diff;
}

