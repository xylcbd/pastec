#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <iostream>
#include <fstream>
#include <list>

#include <opencv2/core/core.hpp>

#include "orbindex.h"
#include "orbwordindex.h"
#include "featureextractor.h"

class ClientConnection;


using namespace cv;
using namespace std;


class ORBFeatureExtractor : public FeatureExtractor
{
public:
    ORBFeatureExtractor(ORBIndex *index, ORBWordIndex *wordIndex);
    virtual ~ORBFeatureExtractor() {}

    u_int32_t processNewImage(unsigned i_imageId, unsigned i_imgSize,
                              char *p_imgData);

private:
    ORBIndex *index;
    ORBWordIndex *wordIndex;
};

#endif // IMAGEPROCESSOR_H
