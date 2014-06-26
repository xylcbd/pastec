#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <opencv2/core/core.hpp>


using namespace std;
using namespace cv;

class ImageLoader
{
public:
    static u_int32_t loadImage(unsigned i_imgSize, char *p_imgData, Mat &img);
};

#endif // IMAGELOADER_H
