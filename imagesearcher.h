#ifndef IMAGESEARCHER_H
#define IMAGESEARCHER_H

#include <queue>

#include <opencv2/core/core.hpp>
#include <opencv2/flann/flann.hpp>

#include "index.h"
#include "wordindex.h"
#include "searchResult.h"
#include "imagereranker.h"

using namespace cv;
using namespace std;

class ClientConnection;


struct SearchRequest
{
    vector<char *> imageData;
    ClientConnection *client;
};


class ImageSearcher
{
public:
    ImageSearcher(Index *index, WordIndex *wordIndex);
    virtual ~ImageSearcher();
    void searchImage(SearchRequest request);

private:
    void returnResults(priority_queue<SearchResult> &rankedResults,
                       SearchRequest &req, unsigned i_maxNbResults);
    void writeResultsHTML(priority_queue<SearchResult> &rankedResults,
                          string htmlPagePath, unsigned i_maxNbResults);
    unsigned long getTimeDiff(const timeval t1, const timeval t2) const;
    void sendResultMsg(SearchRequest &req, list<u_int32_t> &imageIds) const;

    Index *index;
    WordIndex *wordIndex;
    ImageReranker reranker;
};

#endif // IMAGESEARCHER_H
