#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <iostream>
#include <fstream>
#include <list>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>

#include "index.h"
#include "wordindex.h"

class ClientConnection;


using namespace cv;
using namespace std;


class ImageFeatureExtractor
{
public:
    ImageFeatureExtractor(Index *index, WordIndex *wordIndex);

    bool processNewImage(unsigned i_imageId, unsigned i_imgSize,
                         char *p_imgData, ClientConnection *p_client);

private:
    bool openHitFile(ofstream &ofs, unsigned i_imageId);
    bool writeHit(ofstream &ofs, HitForward hit);

    Index *index;
    WordIndex *wordIndex;
};

#endif // IMAGEPROCESSOR_H
