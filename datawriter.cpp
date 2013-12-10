#include "datawriter.h"


DataWriter::DataWriter(string fileName)
    : fileName(fileName)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&hitAvailable, NULL);
}


DataWriter::~DataWriter()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&hitAvailable);
}


/**
 * @brief DataWriter main thread.
 * @return a null pointer.
 */
void *DataWriter::run()
{
    ofs.open(fileName.c_str(), ios_base::app | ios_base::binary);

    while (!b_mustStop)
    {
        pthread_mutex_lock(&mutex);
        if (hits.empty())
        {
            pthread_cond_wait(&hitAvailable, &mutex);
            if (hits.empty())
            {
                pthread_mutex_unlock(&mutex);
                continue;
            }
        }

        Hit hit = hits.front();
        hits.pop();

        pthread_mutex_unlock(&mutex);

        assert(writeToFile(hit));
    }

    closeFile();

    return NULL;
}


/**
 * @brief Add a hit in the processing queue for being writen.
 * @param hit the hit to write.
 */
void DataWriter::queueHit(Hit hit)
{
    pthread_mutex_lock(&mutex);

    hits.push(hit);
    pthread_cond_signal(&hitAvailable);

    pthread_mutex_unlock(&mutex);
}


/**
 * @brief Stop the data writer.
 */
void DataWriter::stop()
{
    pthread_mutex_lock(&mutex);

    b_mustStop = true;
    pthread_cond_signal(&hitAvailable);

    pthread_mutex_unlock(&mutex);

    join();
}


/**
 * @brief Write a new hit in the file.
 * @param hit the new hit to write.
 * @return true on success else false.
 */
bool DataWriter::writeToFile(Hit hit)
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


/**
 * @brief Close the foward index file.
 */
void DataWriter::closeFile()
{
    if (ofs.good())
    {
        // Write the remaining hits.
        while (!hits.empty())
        {
            Hit hit = hits.front();
            hits.pop();
            writeToFile(hit);
        }
        ofs.flush();
        ofs.close();
    }
}
