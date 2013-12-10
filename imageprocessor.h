#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <list>

#include <opencv2/core/core.hpp>
#include <opencv2/nonfree/features2d.hpp>

class DataWriter;


using namespace cv;


class ImageProcessor
{
public:
    ImageProcessor(DataWriter *dataWriter, String visualWordsPath);
    ~ImageProcessor();

    bool processNewImage(unsigned i_imageId, unsigned i_imgSize, char *p_imgData);

private:
    bool readVisualWords(string fileName);

    DataWriter *dataWriter;
    Mat *words;  // The matrix that stores the visual words.
    flann::Index *index; // The kd-tree index.
};

#endif // IMAGEPROCESSOR_H
