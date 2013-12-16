#include <iostream>
#include <cstdlib>
#include <sys/time.h>

#include "backwardindexreader.h"


BackwardIndexReader::BackwardIndexReader(string backwardIndexPath)
{
    // Open the file.
    ifs.open(backwardIndexPath.c_str(), ios_base::binary);
    if (!ifs.good())
    {
        cout << "Could not open the backward index file." << endl;
        exit(1);
    }

    /* Read the table to know where are located the lines corresponding to each
     * visual word. */
    cout << "Reading the numbers of occurences." << endl;
    wordOffSet = new u_int64_t[NB_VISUAL_WORDS];
    nbOccurences = new u_int64_t[NB_VISUAL_WORDS];
    u_int64_t i_offset = NB_VISUAL_WORDS * sizeof(u_int64_t);
    for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
    {
        ifs.read((char *)(nbOccurences + i), sizeof(u_int64_t));
        wordOffSet[i] = i_offset;
        i_offset += nbOccurences[i] * BACKWARD_INDEX_ENTRY_SIZE;
    }

    /* Count the number of words per image. */
    cout << "Count the number of words per image." << endl;
    unsigned i_totalNbRecords = 0;
    while (!ifs.eof())
    {
        u_int32_t i_imageId;
        u_int16_t i_angle, x, y;
        ifs.read((char *)&i_imageId, sizeof(u_int32_t));
        ifs.read((char *)&i_angle, sizeof(u_int16_t));
        ifs.read((char *)&x, sizeof(u_int16_t));
        ifs.read((char *)&y, sizeof(u_int16_t));
        nbWords[i_imageId]++;
        i_totalNbRecords++;
    }

    maxNbRecords = 0.0001 * i_totalNbRecords;
    unsigned i_nbSkipedWords = 0;
    for (unsigned i = 0; i < NB_VISUAL_WORDS; ++i)
        if (nbOccurences[i] > maxNbRecords)
            i_nbSkipedWords++;

    cout << "Nb skipped words: " << i_nbSkipedWords << endl;

    ifs.clear();
}


/**
 * @brief Return the number of occurences of a word in an whole index.
 * @param i_wordId the word id.
 * @return the number of occurences.
 */
unsigned BackwardIndexReader::getWordNbOccurences(unsigned i_wordId)
{
    return nbOccurences[i_wordId];
}


BackwardIndexReader::~BackwardIndexReader()
{
    delete[] wordOffSet;
    delete[] nbOccurences;
}


void BackwardIndexReader::getImagesWithVisualWords(map<u_int32_t, list<Hit> > &imagesReqHits,
                                                   map<u_int32_t, vector<Hit> > &indexHits)
{
    /* We assume that the map is ordered from the lowest key to the highest
     * to read the index continuously. */
    for (map<u_int32_t, list<Hit> >::const_iterator it = imagesReqHits.begin();
         it != imagesReqHits.end(); ++it)
    {
        const unsigned i_wordId = it->first;
        ifs.seekg(wordOffSet[i_wordId]);
        vector<Hit> &hits = indexHits[i_wordId];

        const unsigned i_nbOccurences = nbOccurences[i_wordId];
        hits.resize(i_nbOccurences);

        for (u_int64_t i = 0; i < i_nbOccurences; ++i)
        {
            u_int32_t i_imageId;
            u_int16_t i_angle, x, y;
            ifs.read((char *)&i_imageId, sizeof(u_int32_t));
            ifs.read((char *)&i_angle, sizeof(u_int16_t));
            ifs.read((char *)&x, sizeof(u_int16_t));
            ifs.read((char *)&y, sizeof(u_int16_t));
            hits[i].i_imageId = i_imageId;
            hits[i].i_angle = i_angle;
            hits[i].x = x;
            hits[i].y = y;
        }
    }
}


unsigned BackwardIndexReader::countTotalNbWord(unsigned i_imageId)
{
    return nbWords[i_imageId];
}


unsigned BackwardIndexReader::getTotalNbIndexedImages()
{
    return nbWords.size();
}
