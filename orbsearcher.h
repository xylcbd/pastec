#ifndef IMAGESEARCHER_H
#define IMAGESEARCHER_H

#include <queue>

#include <opencv2/core/core.hpp>
#include <opencv2/flann/flann.hpp>

#include "orbindex.h"
#include "wordindex.h"
#include "searchResult.h"
#include "imagereranker.h"

using namespace cv;
using namespace std;

class ClientConnection;


struct SearchRequest
{
    vector<char> imageData;
    ClientConnection *client;
};


class ORBSearcher
{
public:
    ORBSearcher(ORBIndex *index, WordIndex *wordIndex);
    virtual ~ORBSearcher();
    u_int32_t searchImage(SearchRequest request);

private:
    void returnResults(priority_queue<SearchResult> &rankedResults,
                       SearchRequest &req, unsigned i_maxNbResults);
    unsigned long getTimeDiff(const timeval t1, const timeval t2) const;
    void sendResultMsg(SearchRequest &req, list<u_int32_t> &imageIds) const;

    ORBIndex *index;
    WordIndex *wordIndex;
    ImageReranker reranker;
};

#endif // IMAGESEARCHER_H
