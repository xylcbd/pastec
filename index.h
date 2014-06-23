#ifndef BACKWARDINDEXREADER_H
#define BACKWARDINDEXREADER_H

#include <fstream>
#include <string>

#include <sys/types.h>

#include <map>
#include <vector>
#include <list>
#include <tr1/unordered_map>

#include "hit.h"
#include "backwardindexreaderaccess.h"

using namespace std;
using namespace std::tr1;


#define NB_VISUAL_WORDS 1000000
#define BACKWARD_INDEX_ENTRY_SIZE 10

/* Minimum of the number of hits before a visual word
gets filtered out. */
#define MIN_TOTAL_NB_HITS_FOR_FILTERING_OUT 1000 * 10000

class Index
{
public:
    Index(string backwardIndexPath);
    ~Index();
    void getImagesWithVisualWords(unordered_map<u_int32_t, list<Hit> > &imagesReqHits,
                                  unordered_map<u_int32_t, vector<Hit> > &indexHitsForReq);
    unsigned getWordNbOccurences(unsigned i_wordId);
    unsigned countTotalNbWord(unsigned i_imageId);
    unsigned getTotalNbIndexedImages();
    unsigned getMaxNbRecords() const {return maxNbRecords;}
    u_int32_t addImage(unsigned i_imageId, list<HitForward> hitList);
    bool removeImage(const unsigned i_imageId);
    bool write();
    bool clear();

private:
    bool readIndex();
    bool openHitFile(ofstream &ofs, unsigned i_imageId);
    bool writeHit(ofstream &ofs, HitForward hit);

    string mBackwardIndexPath;
    u_int64_t nbOccurences[NB_VISUAL_WORDS];
    unsigned maxNbRecords;
    u_int64_t totalNbRecords;

    unordered_map<u_int64_t, unsigned> nbWords;
    vector<Hit> indexHits[NB_VISUAL_WORDS];

    BackwardIndexReaderAccess *indexAccess;

    pthread_mutex_t readMutex;
};

#endif // BACKWARDINDEXREADER_H
