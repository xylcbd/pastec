#ifndef BACKWARDINDEXBUILDER_H
#define BACKWARDINDEXBUILDER_H

#include <sys/types.h>
#include <fstream>

#include "thread.h"

using namespace std;


#define NB_VISUAL_WORDS 1000000
#define NB_HIT_BUCKET 500000
#define HIT_DATA_SIZE 12
#define BACKWARD_INDEX_ENTRY_SIZE 8


class BackwardIndexBuilder : public Thread
{
public:
    BackwardIndexBuilder(string forwardIndexPath,
                         string backwardIndexPath);
    void init();
    virtual ~BackwardIndexBuilder();
    void *run();
    void countWriteVisualWordOccurrences();
    void writeIndex();
    void writeIndexBucket(unsigned i_nbHits);
    bool hasFinished();

private:
    void quickSortBucket(char *p_buf, unsigned start, unsigned end);
    u_int32_t getHitWordId(char *p_buf, unsigned i);

    ifstream ifs;
    ofstream ofs;

    u_int64_t *wordOffSet;

    pthread_mutex_t mutexCompleted;
    bool b_completed;

    string forwardIndexPath;
    string backwardIndexPath;
};

#endif // BACKWARDINDEXBUILDER_H
