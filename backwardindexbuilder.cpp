#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include "backwardindexbuilder.h"


BackwardIndexBuilder::BackwardIndexBuilder(string forwardIndexPath,
                                           string backwardIndexPath)
    : wordOffSet(NULL),
      forwardIndexPath(forwardIndexPath),
      backwardIndexPath(backwardIndexPath)
{
    pthread_mutex_init(&mutexCompleted, NULL);
}


void BackwardIndexBuilder::init()
{
    ifs.open(forwardIndexPath.c_str(), ios_base::binary);
    if (!ifs.good())
    {
        cout << "Could not open the forward index file." << endl;
        exit(1);
    }

    ofs.open(backwardIndexPath.c_str(), ios_base::binary);
    if (!ofs.good())
    {
        cout << "Could not open the backward index file." << endl;
        exit(1);
    }

    wordOffSet = new u_int64_t[NB_VISUAL_WORDS + 1];
}


BackwardIndexBuilder::~BackwardIndexBuilder()
{
    ifs.close();
    ofs.close();
    delete[] wordOffSet;
    pthread_mutex_destroy(&mutexCompleted);
}


/**
 * @brief BackwardIndexBuilder main thread.
 * @return a null pointer.
 */
void *BackwardIndexBuilder::run()
{
    pthread_mutex_lock(&mutexCompleted);
    b_completed = false;
    pthread_mutex_unlock(&mutexCompleted);

    countWriteVisualWordOccurrences();
    writeIndex();

    pthread_mutex_lock(&mutexCompleted);
    b_completed = true;
    pthread_mutex_unlock(&mutexCompleted);

    return NULL;
}


void BackwardIndexBuilder::countWriteVisualWordOccurrences()
{
    u_int64_t *nbOccurences = new u_int64_t[NB_VISUAL_WORDS];

    cout << "Couting the number of occurrences of each visual word." << endl;
    while (!ifs.eof())
    {
        // Read one hit.
        char p_buf[12];
        ifs.read(p_buf, 12);

        u_int32_t i_wordId = *(u_int32_t *)(p_buf);

        // Increment the counter for the current word.
        nbOccurences[i_wordId]++;
    }

    cout << "Writing the number of occurences." << endl;
    u_int64_t i_offset = NB_VISUAL_WORDS * sizeof(u_int64_t);
    for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
    {
        ofs.write((char *)(nbOccurences + i), sizeof(u_int64_t));
        wordOffSet[i] = i_offset;
        i_offset += nbOccurences[i] * BACKWARD_INDEX_ENTRY_SIZE;
    }
    /* Word n°NB_VISUAL_WORDS does not exist but we store its offset
     * for later boundary checks. */
    wordOffSet[NB_VISUAL_WORDS] = i_offset;

    delete[] nbOccurences;
}


void BackwardIndexBuilder::writeIndex()
{
    cout << "Writing the backward index." << endl;

    // Count the number of hits in the forward index.
    ifs.clear();
    ifs.seekg(0, ios_base::end);
    unsigned long i_forwardIndexNbHits = ifs.tellg() / HIT_DATA_SIZE;
    cout << i_forwardIndexNbHits << " hits in the forward index." << endl;
    // Return at the begining of the forward index.
    ifs.seekg(0, ios_base::beg);

    // Writing all the forward index buckets.
    unsigned long i = 0;
    while ((i + 1) * NB_HIT_BUCKET < i_forwardIndexNbHits)
    {
        cout << "Writing bucket n°" << i << "." << endl;
        writeIndexBucket(NB_HIT_BUCKET);
        i++;
    }

    cout << "Writing last bucket." << endl;
    writeIndexBucket(i_forwardIndexNbHits - i * NB_HIT_BUCKET);
    cout << "Done !" << endl;
}


void BackwardIndexBuilder::writeIndexBucket(unsigned i_nbHits)
{
    // Store in memory the portion of the file corresponding to the current bucket.
    char *p_buf = new char[i_nbHits * HIT_DATA_SIZE];
    ifs.read(p_buf, i_nbHits * HIT_DATA_SIZE);

#if 1
    cout << "Quick sort the bucket." << endl;
    quickSortBucket(p_buf, 0, i_nbHits - 1);
    cout << "Write." << endl;
#endif

    char *p = p_buf;
    for (unsigned i = 0; i < i_nbHits; ++i)
    {
        // Read one hit from the buffer.
        u_int32_t i_wordId = *(u_int32_t *)p;
        p += sizeof(u_int32_t);
        u_int32_t i_imageId = *(u_int32_t *)p;
        p += sizeof(u_int32_t);
        u_int32_t i_bundledFeatureId = *(u_int32_t *)p;
        p += sizeof(u_int32_t);

        // Write the hit at the right place of the backward index.
        ofs.seekp(wordOffSet[i_wordId]);
        ofs.write((char *)(&i_imageId), sizeof(u_int32_t));
        ofs.write((char *)(&i_bundledFeatureId), sizeof(u_int32_t));

        wordOffSet[i_wordId] += BACKWARD_INDEX_ENTRY_SIZE;

        // Check that we don't write the data over the range of the next word.
        assert(wordOffSet[i_wordId] <= wordOffSet[i_wordId + 1]);
    }

    delete[] p_buf;
}


void BackwardIndexBuilder::quickSortBucket(char *p_buf, unsigned start, unsigned end)
{
    if (start < end)
    {
        u_int32_t pivot = getHitWordId(p_buf, end);
        char pivotData[HIT_DATA_SIZE];
        memcpy(pivotData, p_buf + end * HIT_DATA_SIZE, HIT_DATA_SIZE);

        unsigned i = start;
        unsigned j = end;
        while (i != j)
        {
            if (getHitWordId(p_buf, i) < pivot)
                i++;
            else
            {
                memcpy(p_buf + j * HIT_DATA_SIZE, p_buf + i * HIT_DATA_SIZE, HIT_DATA_SIZE);
                memcpy(p_buf + i * HIT_DATA_SIZE, p_buf + (j - 1) * HIT_DATA_SIZE, HIT_DATA_SIZE);
                j--;
            }
        }
        memcpy(p_buf + j * HIT_DATA_SIZE, pivotData, HIT_DATA_SIZE);
        if (j > 0)
            quickSortBucket(p_buf, start, j - 1);
        quickSortBucket(p_buf, j + 1, end);
    }
}


u_int32_t BackwardIndexBuilder::getHitWordId(char *p_buf, unsigned i)
{
    return *(u_int32_t *)(p_buf + i * HIT_DATA_SIZE);
}


/**
 * @brief Tell if the backward indexing has finished.
 * @return true if finished else false.
 */
bool BackwardIndexBuilder::hasFinished()
{
    pthread_mutex_lock(&mutexCompleted);
    bool b_ret = b_completed;
    pthread_mutex_unlock(&mutexCompleted);
    return b_ret;
}
