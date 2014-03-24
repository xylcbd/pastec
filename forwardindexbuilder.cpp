#include <sstream>

#include "forwardindexbuilder.h"


ForwardIndexBuilder::ForwardIndexBuilder(string fileName)
    : fileName(fileName)
{ }


/**
 * @brief DataWriter main thread.
 * @return a null pointer.
 */
bool ForwardIndexBuilder::build(unsigned i_nbImages, char *p_imageIds)
{
    cout << "Build the forward index containing " << i_nbImages
         << " images." << endl;

    ofs.open(fileName.c_str(), ios_base::binary);

    for (unsigned i = 0; i < i_nbImages; ++i)
    {
        unsigned i_imageId = *(u_int32_t *)p_imageIds;
        addImage(i_imageId);
        p_imageIds += sizeof(u_int32_t);
    }

    ofs.close();

    return true;
}


/**
 * @brief Add the hits of an image to the forward index.
 * @param i_imageId the image id.
 * @return true on success else false.
 */
bool ForwardIndexBuilder::addImage(unsigned i_imageId)
{
    stringstream fileNameStream;
    fileNameStream << "imageHits/" << i_imageId << ".dat";

    ifstream ifs;
    // Open the file and place the cursor at the end of the file.
    ifs.open(fileNameStream.str().c_str(), ios_base::binary | ios_base::ate);
    if (!ifs.good())
    {
        cout << "Could not open the hit file of image " << i_imageId
             << "." << endl;
        return false;
    }

    // Read the file length.
    long i_fileLength = ifs.tellg();
    // Go at the begining of the file.
    ifs.seekg(0);
    ifs.clear();

    // Save the file in memory.
    char *p_fileData = new char[i_fileLength];
    if (p_fileData == NULL)
    {
        cout << "Could not load the hit file of image " << i_imageId
             << " in memory." << endl;
        return false;
    }
    ifs.read(p_fileData, i_fileLength);
    ifs.close();

    // Write the file in the forward index.
    ofs.write(p_fileData, i_fileLength);

    return true;
}
