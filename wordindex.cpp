#include <iostream>
#include <fstream>

#include "wordindex.h"


WordIndex::WordIndex(string visualWordsPath)
{
    words = new Mat(0, 32, CV_8U); // The matrix that stores the visual words.

    if (!readVisualWords(visualWordsPath))
        exit(1);
    assert(words->rows == 1000000);

    cout << "Building the word index." << endl;

    cvflann::Matrix<unsigned char> m_features
            ((unsigned char*)words->ptr<unsigned char>(0), words->rows, words->cols);
    kdIndex = new cvflann::HierarchicalClusteringIndex<cvflann::Hamming<unsigned char> >
            (m_features,cvflann::HierarchicalClusteringIndexParams(10));
    kdIndex->buildIndex();
}


WordIndex::~WordIndex()
{
    delete words;
    delete kdIndex;
}


void WordIndex::knnSearch(const Mat& query, vector<int>& indices,
                          vector<int>& dists, int knn)
{
    cvflann::KNNResultSet<int> m_indices(knn);

    m_indices.init(indices.data(), dists.data());

    kdIndex->findNeighbors(m_indices, (unsigned char*)query.ptr<unsigned char>(0),
                           cvflann::SearchParams(1));
}


/**
 * @brief Read the list of visual words from an external file.
 * @param fileName the path of the input file name.
 * @param words a pointer to a matrix to store the words.
 * @return true on success else false.
 */
bool WordIndex::readVisualWords(string fileName)
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

    unsigned char c;
    while (ifs.good())
    {
        Mat line(1, 32, CV_8U);
        for (unsigned i_col = 0; i_col < 32; ++i_col)
        {
            ifs.read((char*)&c, sizeof(unsigned char));
            line.at<unsigned char>(0, i_col) = c;
        }
        if (!ifs.good())
            break;
        words->push_back(line);
    }

    ifs.close();

    return true;
}
