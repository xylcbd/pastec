#include <iostream>
#include <cstdlib>
#include <sys/time.h>

#include "index.h"


Index::Index(string backwardIndexPath)
    : mBackwardIndexPath(backwardIndexPath),
      maxNbRecords(1000000)
{
    // Init the mutex.
    pthread_mutex_init(&readMutex, NULL);

    // Initialize the nbOccurences table.
    for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
        nbOccurences[i] = 0;

    readIndex();
}


/**
 * @brief Return the number of occurences of a word in an whole index.
 * @param i_wordId the word id.
 * @return the number of occurences.
 */
unsigned Index::getWordNbOccurences(unsigned i_wordId)
{
    return nbOccurences[i_wordId];
}


Index::~Index()
{
    indexAccess->close();
    delete indexAccess;
    pthread_mutex_destroy(&readMutex);
}


void Index::getImagesWithVisualWords(unordered_map<u_int32_t, list<Hit> > &imagesReqHits,
                                     unordered_map<u_int32_t, vector<Hit> > &indexHitsForReq)
{
    pthread_mutex_lock(&readMutex);

    for (unordered_map<u_int32_t, list<Hit> >::const_iterator it = imagesReqHits.begin();
         it != imagesReqHits.end(); ++it)
    {
        const unsigned i_wordId = it->first;
        indexHitsForReq[i_wordId] = indexHits[i_wordId];
    }

    pthread_mutex_unlock(&readMutex);
}


unsigned Index::countTotalNbWord(unsigned i_imageId)
{
    return nbWords[i_imageId];
}


unsigned Index::getTotalNbIndexedImages()
{
    return nbWords.size();
}


/**
 * @brief Add a list of hits to the index.
 * @param  the list of hits.
 */
void Index::addImage(list<HitForward> hitList)
{
    for (list<HitForward>::iterator it = hitList.begin(); it != hitList.end(); ++it)
    {
        HitForward hitFor = *it;
        Hit hitBack;
        hitBack.i_imageId = hitFor.i_imageId;
        hitBack.i_angle = hitFor.i_angle;
        hitBack.x = hitFor.x;
        hitBack.y = hitFor.y;

        indexHits[hitFor.i_wordId].push_back(hitBack);
        nbWords[hitFor.i_imageId]++;
        nbOccurences[hitFor.i_wordId]++;
        totalNbRecords++;
    }

    if (totalNbRecords > MIN_TOTAL_NB_HITS_FOR_FILTERING_OUT)
        maxNbRecords = 0.00001 * totalNbRecords;
}


/**
 * @brief Read the index and store it in memory.
 * @return true on success else false
 */
bool Index::readIndex()
{
    // Open the file.
    indexAccess = new BackwardIndexReaderFileAccess();
    if (!indexAccess->open(mBackwardIndexPath))
    {
        cout << "Could not open the backward index file." << endl
             << "Using an empty index." << endl;
        return false;
    }
    else
    {
        /* Read the table to know where are located the lines corresponding to each
         * visual word. */
        cout << "Reading the numbers of occurences." << endl;
        u_int64_t *wordOffSet = new u_int64_t[NB_VISUAL_WORDS];
        u_int64_t i_offset = NB_VISUAL_WORDS * sizeof(u_int64_t);
        for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
        {
            indexAccess->read((char *)(nbOccurences + i), sizeof(u_int64_t));
            wordOffSet[i] = i_offset;
            i_offset += nbOccurences[i] * BACKWARD_INDEX_ENTRY_SIZE;
        }

        /* Count the number of words per image. */
        cout << "Count the number of words per image." << endl;
        totalNbRecords = 0;
        while (!indexAccess->endOfIndex())
        {
            u_int32_t i_imageId;
            u_int16_t i_angle, x, y;
            indexAccess->read((char *)&i_imageId, sizeof(u_int32_t));
            indexAccess->read((char *)&i_angle, sizeof(u_int16_t));
            indexAccess->read((char *)&x, sizeof(u_int16_t));
            indexAccess->read((char *)&y, sizeof(u_int16_t));
            nbWords[i_imageId]++;
            totalNbRecords++;
        }

        if (totalNbRecords > MIN_TOTAL_NB_HITS_FOR_FILTERING_OUT)
            maxNbRecords = 0.00001 * totalNbRecords;
        unsigned i_nbSkipedWords = 0;
        for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
            if (nbOccurences[i] > maxNbRecords)
                i_nbSkipedWords++;

        cout << "Nb skipped words: " << i_nbSkipedWords << endl;

        indexAccess->reset();

        cout << "Loading the index in memory" << endl;

        for (unsigned i_wordId = 0; i_wordId < NB_VISUAL_WORDS; ++i_wordId)
        {
            indexAccess->moveAt(wordOffSet[i_wordId]);
            vector<Hit> &hits = indexHits[i_wordId];

            const unsigned i_nbOccurences = nbOccurences[i_wordId];
            hits.resize(i_nbOccurences);

            for (u_int64_t i = 0; i < i_nbOccurences; ++i)
            {
                u_int32_t i_imageId;
                u_int16_t i_angle, x, y;
                indexAccess->read((char *)&i_imageId, sizeof(u_int32_t));
                indexAccess->read((char *)&i_angle, sizeof(u_int16_t));
                indexAccess->read((char *)&x, sizeof(u_int16_t));
                indexAccess->read((char *)&y, sizeof(u_int16_t));
                hits[i].i_imageId = i_imageId;
                hits[i].i_angle = i_angle;
                hits[i].x = x;
                hits[i].y = y;
            }
        }

        delete[] wordOffSet;

        cout << "Done" << endl;
        return true;
    }
}


/**
 * @brief Write the index in memory to a file.
 * @return true on success else false
 */
bool Index::write()
{
    ofstream ofs;

    ofs.open(mBackwardIndexPath.c_str(), ios_base::binary);
    if (!ofs.good())
    {
        cout << "Could not open the backward index file." << endl;
        return false;
    }

    cout << "Writing the number of occurences." << endl;
    for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
        ofs.write((char *)(nbOccurences + i), sizeof(u_int64_t));

    cout << "Writing the index hits." << endl;
    for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
    {
        const vector<Hit> &wordHits = indexHits[i];

        for (unsigned j = 0; j < wordHits.size(); ++j)
        {
            const Hit &hit = wordHits[j];
            ofs.write((char *)(&hit.i_imageId), sizeof(u_int32_t));
            ofs.write((char *)(&hit.i_angle), sizeof(u_int16_t));
            ofs.write((char *)(&hit.x), sizeof(u_int16_t));
            ofs.write((char *)(&hit.y), sizeof(u_int16_t));
        }
    }

    return true;
}


/**
 * @brief Clear the index.
 * @return true on success else false.
 */
bool Index::clear()
{
    // Reset the nbOccurences table.
    for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
    {
        nbOccurences[i] = 0;
        indexHits[i].clear();
    }

    nbWords.clear();
    totalNbRecords = 0;

    return true;
}
