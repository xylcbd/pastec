#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <iostream>
#include <fstream>
#include <list>

#include <opencv2/core/core.hpp>

#include "orbindex.h"
#include "wordindex.h"

class ClientConnection;


using namespace cv;
using namespace std;


class ORBFeatureExtractor
{
public:
    ORBFeatureExtractor(ORBIndex *index, WordIndex *wordIndex);

    u_int32_t processNewImage(unsigned i_imageId, unsigned i_imgSize,
                              char *p_imgData);

private:
    ORBIndex *index;
    WordIndex *wordIndex;
};

#endif // IMAGEPROCESSOR_H
