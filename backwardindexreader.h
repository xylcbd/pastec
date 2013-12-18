#ifndef BACKWARDINDEXREADER_H
#define BACKWARDINDEXREADER_H

#include <fstream>
#include <string>

#include <sys/types.h>

#include <map>
#include <vector>
#include <list>

#include "hit.h"
#include "backwardindexreaderaccess.h"

using namespace std;


#define NB_VISUAL_WORDS 1000000
#define BACKWARD_INDEX_ENTRY_SIZE 10

class BackwardIndexReader
{
public:
    BackwardIndexReader(string backwardIndexPath);
    ~BackwardIndexReader();
    void getImagesWithVisualWords(map<u_int32_t, list<Hit> > &imagesReqHits,
                                  map<u_int32_t, vector<Hit> > &indexHits);
    unsigned getWordNbOccurences(unsigned i_wordId);
    unsigned countTotalNbWord(unsigned i_imageId);
    unsigned getTotalNbIndexedImages();
    unsigned getMaxNbRecords() const {return maxNbRecords;}

private:
    u_int64_t *wordOffSet;
    u_int64_t *nbOccurences;
    unsigned maxNbRecords;

    map<u_int64_t, unsigned> nbWords;

    BackwardIndexReaderAccess *indexAccess;
};

#endif // BACKWARDINDEXREADER_H
