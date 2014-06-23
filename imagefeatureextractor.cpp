#include <iostream>
#include <set>
#include <tr1/unordered_set>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "imagefeatureextractor.h"
#include "clientconnection.h"
#include "dataMessages.h"


ImageFeatureExtractor::ImageFeatureExtractor(Index *index, WordIndex *wordIndex)
    : index(index), wordIndex(wordIndex)
{ }


bool ImageFeatureExtractor::processNewImage(unsigned i_imageId, unsigned i_imgSize,
                                            char *p_imgData, ClientConnection *p_client)
{
    vector<char> imgData(i_imgSize);
    memcpy(imgData.data(), p_imgData, i_imgSize);

    Mat img;
    try
    {
        img = imdecode(imgData, CV_LOAD_IMAGE_GRAYSCALE);
    }
    catch (cv::Exception& e) // The decoding of an image can raise an exception.
    {
        const char* err_msg = e.what();
        cout << "Exception caught: " << err_msg << endl;
        p_client->sendReply(IMAGE_NOT_DECODED);
        return false;
    }

    if (!img.data)
    {
        cout << "Error reading the image." << std::endl;
        p_client->sendReply(IMAGE_NOT_DECODED);
        return false;
    }

    unsigned i_imgWidth = img.cols;
    unsigned i_imgHeight = img.rows;


    if (i_imgWidth > 2000
        || i_imgHeight > 2000)
    {
        cout << "Image too large." << endl;
        p_client->sendReply(IMAGE_SIZE_TOO_BIG);
        return false;
    }

#if 1
    if (i_imgWidth < 200
        || i_imgHeight < 200)
    {
        cout << "Image too small." << endl;
        p_client->sendReply(IMAGE_SIZE_TOO_SMALL);
        return false;
    }
#endif

    vector<KeyPoint> keypoints;
    Mat descriptors;

    ORB(1000, 1.02, 100)(img, noArray(), keypoints, descriptors);

    // Output the SIFT keypoints to the database.

    // TODO: create the imageHits directory if it does not exist.

    ofstream ofs;
    if (!openHitFile(ofs, i_imageId))
    {
        p_client->sendReply(ERROR_GENERIC);
        return false;
    }

    unsigned i_nbKeyPoints = 0;
    list<HitForward> imageHits;
    unordered_set<u_int32_t> matchedWords;
    for (unsigned i = 0; i < keypoints.size(); ++i)
    {
        i_nbKeyPoints++;

        // Recording the angle on 16 bits.
        u_int16_t angle = keypoints[i].angle / 360 * (1 << 16);
        u_int16_t x = keypoints[i].pt.x;
        u_int16_t y = keypoints[i].pt.y;

        vector<int> indices(1);
        vector<int> dists(1);
        wordIndex->knnSearch(descriptors.row(i), indices, dists, 1);

        for (unsigned j = 0; j < indices.size(); ++j)
        {
            const unsigned i_wordId = indices[j];
            if (matchedWords.find(i_wordId) == matchedWords.end())
            {
                HitForward newHit;
                newHit.i_wordId = i_wordId;
                newHit.i_imageId = i_imageId;
                newHit.i_angle = angle;
                newHit.x = x;
                newHit.y = y;
                // Write the hit in the file.
                if (!writeHit(ofs, newHit))
                {
                    ofs.close();
                    p_client->sendReply(ERROR_GENERIC);
                    return false;
                }
                imageHits.push_back(newHit);
                matchedWords.insert(i_wordId);
            }
        }
    }
    // Record the hits.
    index->addImage(imageHits);


    ofs.close();

#if 0
    // Draw keypoints.
    Mat img_res;
    drawKeypoints(img, keypoints, img_res, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

    // Show the image.
    imshow("Keypoints 1", img_res);
    waitKey();
#endif

    p_client->sendReply(OK);

    return true;
}


/**
 * @brief Open the file that will contain all hits of the image.
 * @param i_imageId the image id.
 * @return true on success else false.
 */
bool ImageFeatureExtractor::openHitFile(ofstream &ofs, unsigned i_imageId)
{
    stringstream fileNameStream;
    fileNameStream << "imageHits/" << i_imageId << ".dat";

    ofs.open(fileNameStream.str().c_str(), ios_base::binary);

    if (!ofs.good())
    {
        cout << "Could not open the hit output file." << endl;
        ofs.close();
        return false;
    }

    return true;
}


/**
 * @brief Write a new hit in the file.
 * @param hit the new hit to write.
 * @param ofs the output file stream.
 * @return true on success else false.
 */
bool ImageFeatureExtractor::writeHit(ofstream &ofs, HitForward hit)
{
    if (!ofs.good())
    {
        cout << "Could not write to the output file." << endl;
        return false;
    }

    ofs.write((char *)&hit.i_wordId, sizeof(u_int32_t));
    ofs.write((char *)&hit.i_imageId, sizeof(u_int32_t));
    ofs.write((char *)&hit.i_angle, sizeof(u_int16_t));
    ofs.write((char *)&hit.x, sizeof(u_int16_t));
    ofs.write((char *)&hit.y, sizeof(u_int16_t));

    return true;
}
