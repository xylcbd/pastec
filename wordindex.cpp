#include <iostream>
#include <fstream>

#include "wordindex.h"

WordIndex::WordIndex(string visualWordsPath, string indexPath)
{
    words = new Mat(0, 128, CV_32F); // The matrix that stores the visual words.

    if (!readVisualWords(visualWordsPath))
        exit(1);
    assert(words->rows == 1000000);

    cout << "Building the kd-trees." << endl;
    //index = new flann::Index(*words, flann::KDTreeIndexParams());
    kdIndex = new flann::Index(*words, flann::SavedIndexParams(indexPath));
}


WordIndex::~WordIndex()
{
    delete words;
    delete kdIndex;
}


void WordIndex::knnSearch(const vector<float>& query, vector<int>& indices,
                          vector<float>& dists, int knn)
{
    kdIndex->knnSearch(query, indices, dists, knn);
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
