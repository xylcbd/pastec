#ifndef IMAGERERANKER_H
#define IMAGERERANKER_H

#include <sys/types.h>

#include <queue>
#include <list>
#include <tr1/unordered_map>

#include <opencv2/core/core.hpp>

#include "thread.h"
#include "searchResult.h"
#include "hit.h"

using namespace std;
using namespace std::tr1;
using namespace cv;


class ImageReranker
{
public:
    ImageReranker() {}
    void rerank(map<u_int32_t, list<Hit> > &imagesReqHits,
                map<u_int32_t, vector<Hit> > &indexHits,
                priority_queue<SearchResult> &rankedResultsIn,
                priority_queue<SearchResult> &rankedResultsOut,
                unsigned i_nbResults);
    void rerankRANSAC(unordered_map<u_int32_t, list<Hit> > &imagesReqHits,
                      unordered_map<u_int32_t, vector<Hit> > &indexHits,
                      priority_queue<SearchResult> &rankedResultsIn,
                      priority_queue<SearchResult> &rankedResultsOut,
                      unsigned i_nbResults);
    void rerankNearestNeighbors(map<u_int32_t, list<Hit> > &imagesReqHits,
                                map<u_int32_t, vector<Hit> > &indexHits,
                                priority_queue<SearchResult> &rankedResultsIn,
                                priority_queue<SearchResult> &rankedResultsOut,
                                unsigned i_nbResults);

private:
    float angleDiff(unsigned i_angle1, unsigned i_angle2);
    void getFirstImageIds(priority_queue<SearchResult> &rankedResultsIn,
                          unsigned i_nbResults, set<u_int32_t> &firstImageIds);
};


// A task that must be performed when the rerankRANSAC function is called.
struct RANSACTask
{
    vector<Point2f> points1;
    vector<Point2f> points2;
};


class RANSACThread : public Thread
{
public:
    RANSACThread(pthread_mutex_t *p_itMutex,
                 map<u_int32_t, RANSACTask>::const_iterator &mIt,
                 map<u_int32_t, RANSACTask>::const_iterator &mLastIt,
                 pthread_mutex_t *p_resultsMutex, priority_queue<SearchResult> &rankedResultsOut)
        : p_itMutex(p_itMutex), mIt(mIt), mLastIt(mLastIt),
          p_resultsMutex(p_resultsMutex), rankedResultsOut(rankedResultsOut)
    { }
    virtual ~RANSACThread() {}

    void *run();

private:
    pthread_mutex_t *p_itMutex;
    map<u_int32_t, RANSACTask>::const_iterator &mIt;
    map<u_int32_t, RANSACTask>::const_iterator &mLastIt;

    pthread_mutex_t *p_resultsMutex;
    priority_queue<SearchResult> &rankedResultsOut;
};


// A list of keypoint positions for the rerankNearestNeighbors function.
class PointList : public Mat
{
public:
    PointList() : Mat(0, 2, CV_32S) { }
};


#define HISTOGRAM_NB_BINS 32

struct Histogram
{
    unsigned bins[HISTOGRAM_NB_BINS];
};

#endif // IMAGERERANKER_H
