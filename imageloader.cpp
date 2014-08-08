#include <iostream>
#include <vector>

#include <opencv2/highgui/highgui.hpp>

#include "imageloader.h"
#include "messages.h"


u_int32_t ImageLoader::loadImage(unsigned i_imgSize, char *p_imgData, Mat &img)
{
    vector<char> imgData(i_imgSize);
    memcpy(imgData.data(), p_imgData, i_imgSize);

    try
    {
        img = imdecode(imgData, CV_LOAD_IMAGE_GRAYSCALE);
    }
    catch (cv::Exception& e) // The decoding of an image can raise an exception.
    {
        const char* err_msg = e.what();
        cout << "Exception caught: " << err_msg << endl;
        return IMAGE_NOT_DECODED;
    }

    if (!img.data)
    {
        cout << "Error reading the image." << std::endl;
        return IMAGE_NOT_DECODED;
    }

    unsigned i_imgWidth = img.cols;
    unsigned i_imgHeight = img.rows;


    if (i_imgWidth > 1000
        || i_imgHeight > 1000)
    {
        cout << "Image too large." << endl;
        return IMAGE_SIZE_TOO_BIG;
    }

#if 1
    if (i_imgWidth < 200
        || i_imgHeight < 200)
    {
        cout << "Image too small." << endl;
        return IMAGE_SIZE_TOO_SMALL;
    }
#endif

    return OK;
}
