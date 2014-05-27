#ifndef WORDINDEX_H
#define WORDINDEX_H

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/flann/flann.hpp>

using namespace cv;
using namespace std;


class WordIndex
{
public:
    WordIndex(string visualWordsPath, string indexPath);
    ~WordIndex();
    void knnSearch(const vector<float>& query, vector<int>& indices,
                   vector<float>& dists, int knn);

private:
    bool readVisualWords(string fileName);

    Mat *words;  // The matrix that stores the visual words.
    flann::Index *kdIndex; // The kd-tree index.
};

#endif // WORDINDEX_H
