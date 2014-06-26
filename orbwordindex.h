#ifndef ORBWORDINDEX_H
#define ORBWORDINDEX_H

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/flann/flann.hpp>

using namespace cv;
using namespace std;


class ORBWordIndex
{
public:
    ORBWordIndex(string visualWordsPath);
    ~ORBWordIndex();
    void knnSearch(const Mat &query, vector<int>& indices,
                   vector<int> &dists, int knn);

private:
    bool readVisualWords(string fileName);

    Mat *words;  // The matrix that stores the visual words.
    cvflann::HierarchicalClusteringIndex<cvflann::Hamming<unsigned char> > *kdIndex; // The kd-tree index.
};

#endif // ORBWORDINDEX_H
