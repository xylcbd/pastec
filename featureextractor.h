#ifndef FEATUREEXTRACTOR_H
#define FEATUREEXTRACTOR_H

#include <sys/types.h>


class FeatureExtractor
{
public:
    virtual u_int32_t processNewImage(unsigned i_imageId, unsigned i_imgSize,
                                      char *p_imgData) = 0;
};

#endif // FEATUREEXTRACTOR_H
