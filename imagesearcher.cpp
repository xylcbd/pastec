#include <iostream>
#include <fstream>
#include <sys/time.h>

#include <set>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "imagesearcher.h"
#include "clientconnection.h"
#include "dataMessages.h"


ImageSearcher::ImageSearcher(string backwardIndexPath, string visualWordsPath)
    : backwardIndexPath(backwardIndexPath), visualWordsPath(visualWordsPath)
{ }


/**
 * @brief Init the image searcher.
 */
void ImageSearcher::init()
{
    backwardIndex = new BackwardIndexReader(backwardIndexPath);

    // Index reader initialization.
    words = new Mat(0, 128, CV_32F); // The matrix that stores the visual words.
    if (!readVisualWords(visualWordsPath, words))
    {
        cout << "Can't read the visual word file." << endl;
        exit(1);
    }
    assert(words->rows == 1000000);

    cout << "Building the kd-trees." << endl;
    myIndex = new flann::Index(*words, flann::KDTreeIndexParams());

    cout << "Ready to accept search queries." << endl;
}


/**
 * @brief Stop the image searcher.
 */
void ImageSearcher::stop()
{
    delete myIndex;
    delete words;
    delete backwardIndex;
}


bool ImageSearcher::searchImage(SearchRequest request)
{
    timeval t[5];
    gettimeofday(&t[0], NULL);

    cout << "Loading the image and extracting the SIFTs." << endl;

    Mat img;
    try
    {
        img = imdecode(request.imageData, CV_LOAD_IMAGE_GRAYSCALE);
    }
    catch (cv::Exception& e) // The decoding of an image can raise an exception.
    {
        const char* err_msg = e.what();
        cout << "Exception caught: " << err_msg << endl;
        request.client->sendReply(IMAGE_NOT_DECODED);
        return false;
    }

    if (!img.data)
    {
        cout << "Error reading the image." << std::endl;
        request.client->sendReply(IMAGE_NOT_DECODED);
        return false;
    }

    unsigned i_imgWidth = img.cols;
    unsigned i_imgHeight = img.rows;

    if (i_imgWidth > 2000
        || i_imgHeight > 2000)
    {
        cout << "Image too large." << endl;
        request.client->sendReply(IMAGE_SIZE_TOO_BIG);
        return false;
    }

#if 0
    if (i_imgWidth < 150
        || i_imgHeight < 150)
    {
        cout << "Image too small." << endl;
        return false;
    }
#endif

    vector<KeyPoint> keypoints;
    Mat descriptors;

    SIFT()(img, noArray(), keypoints, descriptors);
    std::cout << "Nb SIFTs: " << keypoints.size() << std::endl;

    gettimeofday(&t[1], NULL);

    cout << "time: " << getTimeDiff(t[0], t[1]) << " ms." << endl;
    cout << "Looking for the visual words. " << endl;

    map<u_int32_t, list<Hit> > imageReqHits; // key: visual word, value: the found angles
    for (unsigned i = 0; i < keypoints.size(); ++i)
    {
        #define NB_NEIGHBORS 4

        vector<int> indices(NB_NEIGHBORS);
        vector<float> dists(NB_NEIGHBORS);
        myIndex->knnSearch(descriptors.row(i), indices,
                           dists, NB_NEIGHBORS);

        for (unsigned j = 0; j < indices.size(); ++j)
        {
            /* If the word has a too large number of occurence in the index, we consider
             * that it is not relevant. */
            if (backwardIndex->getWordNbOccurences(indices[j]) > backwardIndex->getMaxNbRecords())
                continue;

            // Convert the angle to a 16 bit integer.
            Hit hit;
            hit.i_imageId = 0;
            hit.i_angle = keypoints[i].angle / 360 * (1 << 16);
            hit.x = keypoints[i].pt.x / i_imgWidth * (1 << 16);
            hit.y = keypoints[i].pt.y / i_imgHeight * (1 << 16);

            imageReqHits[indices[j]].push_back(hit);
        }
    }

    cout << imageReqHits.size() << " SIFTs kept for the request." << endl;

    const unsigned i_nbTotalIndexedImages = backwardIndex->getTotalNbIndexedImages();
    cout << i_nbTotalIndexedImages << " images indexed in the index." << endl;

    map<u_int32_t, vector<Hit> > indexHits; // key: visual word id, values: index hits.
    backwardIndex->getImagesWithVisualWords(imageReqHits, indexHits);

    gettimeofday(&t[2], NULL);
    cout << "time: " << getTimeDiff(t[1], t[2]) << " ms." << endl;
    cout << "Ranking the images." << endl;

    map<u_int32_t, float> weights; // key: image id, value: image score.

    for (map<u_int32_t, vector<Hit> >::const_iterator it = indexHits.begin();
        it != indexHits.end(); ++it)
    {
        const vector<Hit> &hits = it->second;

        const float f_weight = log((float)i_nbTotalIndexedImages / hits.size());
        for (vector<Hit>::const_iterator it2 = hits.begin();
             it2 != hits.end(); ++it2)
        {
            /* TF-IDF according to the paper "Video Google:
             * A Text Retrieval Approach to Object Matching in Videos" */
            unsigned i_totalNbWords = backwardIndex->countTotalNbWord(it2->i_imageId);
            weights[it2->i_imageId] += f_weight / i_totalNbWords * 4;
        }
    }

    priority_queue<SearchResult> rankedResults;
    for (map<unsigned, float>::iterator it = weights.begin();
         it != weights.end(); ++it)
        rankedResults.push(SearchResult(it->second, it->first));

    gettimeofday(&t[3], NULL);
    cout << "time: " << getTimeDiff(t[2], t[3]) << " ms." << endl;
    cout << "Reranking " << weights.size() << " images." << endl;

    priority_queue<SearchResult> rerankedResults;
    reranker.rerankRANSAC(imageReqHits, indexHits,
                          rankedResults, rerankedResults, 300);

    gettimeofday(&t[4], NULL);
    cout << "time: " << getTimeDiff(t[3], t[4]) << " ms." << endl;
    cout << "Returning the results. " << endl;

    returnResults(rerankedResults, request, 300);

#if 0
    // Draw keypoints and ellipses.
    Mat img_res;
    drawKeypoints(img, cleanKeypoints, img_res, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    for (unsigned i = 0; i < ellipses.size(); ++i)
        ellipse( img_res, ellipses[i], Scalar(0, 0, 255), 1);

    // Show the image.
    imshow("Keypoints 1", img_res);
#endif

    return true;
}


/**
 * @brief Return to the client the found results.
 * @param rankedResults the ranked list of results.
 * @param req the received search request.
 * @param i_maxNbResults the maximum number of results returned.
 */
void ImageSearcher::returnResults(priority_queue<SearchResult> &rankedResults,
                                  SearchRequest &req, unsigned i_maxNbResults)
{
    list<u_int32_t> imageIds;

    unsigned i_res = 0;
    while(!rankedResults.empty()
          && i_res < i_maxNbResults)
    {
        const SearchResult &res = rankedResults.top();
        imageIds.push_back(res.i_imageId);
        i_res++;
        rankedResults.pop();
    }

    sendResultMsg(req, imageIds);
}


/**
 * @brief Read the list of visual words from an external file.
 * @param fileName the path of the input file name.
 * @param words a pointer to a matrix to store the words.
 * @return true on success else false.
 */
bool ImageSearcher::readVisualWords(string fileName, Mat *words)
{
    cout << "Reading the visual words file." << endl;

    // Open the input file.
    ifstream ifs;
    ifs.open(fileName.c_str(), ios_base::binary);

    if (!ifs.good())
    {
        cout << "Could not open the input file." << endl;
        return false;
    }

    float c;
    while (ifs.good())
    {
        Mat line(1, 128, CV_32F);
        for (unsigned i_col = 0; i_col < 128; ++i_col)
        {
            ifs.read((char *)&c, sizeof(float));
            line.at<float>(0, i_col) = c;
        }
        if (!ifs.good())
            break;
        words->push_back(line);
        ifs.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    ifs.close();

    return true;
}


/**
 * @brief Get the time difference in ms between two instants.
 * @param t1
 * @param t2
 */
unsigned long ImageSearcher::getTimeDiff(const timeval t1, const timeval t2) const
{
    return ((t2.tv_sec - t1.tv_sec) * 1000000
            + (t2.tv_usec - t1.tv_usec)) / 1000;
}


/**
 * @brief Send the list of mathched image id to the client.
 * @param imageIds the list of image id results.
 */
void ImageSearcher::sendResultMsg(SearchRequest &req, list<u_int32_t> &imageIds) const
{
    /* Search result:
     * - 1 byte for the command code.
     * - 4 bytes to give the number of images retrived.
     * - 4 bytes per image giving their ids.
     */

    char *msg = new char[1 + 4 + imageIds.size() * sizeof(u_int32_t)];

    *(char *)(msg) = 1; // Result message.
    *(u_int32_t *)(msg + 1) = imageIds.size();

    char *p = msg + 5;
    for (list<unsigned>::iterator it = imageIds.begin();
         it != imageIds.end(); ++it)
    {
        *(u_int32_t *)p = *it;
        p += sizeof(u_int32_t);
    }

    req.client->sendReply(p - msg, msg);
}
