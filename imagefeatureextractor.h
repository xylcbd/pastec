#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <iostream>
#include <fstream>
#include <list>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>

#include "forwardindexbuilder.h"

class ClientConnection;


using namespace cv;
using namespace std;


class ImageFeatureExtractor
{
public:
    ImageFeatureExtractor(String visualWordsPath, String indexPath);
    void init();
    void stop();

    bool processNewImage(unsigned i_imageId, unsigned i_imgSize,
                         char *p_imgData, ClientConnection *p_client);

private:
    bool openHitFile(ofstream &ofs, unsigned i_imageId);
    bool writeHit(ofstream &ofs, HitForward hit);
    bool readVisualWords(string fileName);

    String visualWordsPath;
    String indexPath;
    Mat *words;  // The matrix that stores the visual words.
    flann::Index *index; // The kd-tree index.
};

#endif // IMAGEPROCESSOR_H
