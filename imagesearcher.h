#ifndef IMAGESEARCHER_H
#define IMAGESEARCHER_H

#include <queue>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/flann/flann.hpp>

#include "backwardindexreader.h"
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
    ImageSearcher(string backwardIndexPath, string visualWordsPath,
                  string indexPath);
    virtual ~ImageSearcher();
    void init();
    void stop();
    void searchImage(SearchRequest request);

private:
    void returnResults(priority_queue<SearchResult> &rankedResults,
                       SearchRequest &req, unsigned i_maxNbResults);
    void writeResultsHTML(priority_queue<SearchResult> &rankedResults,
                          string htmlPagePath, unsigned i_maxNbResults);
    bool readVisualWords(string fileName, Mat *words);
    unsigned long getTimeDiff(const timeval t1, const timeval t2) const;
    void sendResultMsg(SearchRequest &req, list<u_int32_t> &imageIds) const;
    float computeSIFTEntropy(unsigned i_word) const;

    BackwardIndexReader *backwardIndex;
    ImageReranker reranker;

    string backwardIndexPath;
    string visualWordsPath;
    string indexPath;

    Mat *words;
    flann::Index *myIndex;
};

#endif // IMAGESEARCHER_H
