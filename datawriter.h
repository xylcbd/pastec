#ifndef DATAWRITER_H
#define DATAWRITER_H

#include <iostream>
#include <sys/types.h>
#include <fstream>
#include <queue>

#include "thread.h"

using namespace std;


struct Hit
{
    u_int32_t i_wordId;
    u_int32_t i_imageId;
    u_int16_t i_angle;
    u_int16_t x;
    u_int16_t y;
};


class DataWriter : public Thread
{
public:
    DataWriter(string fileName);
    virtual ~DataWriter();
    void queueHit(Hit hit);
    void stop();

private:
    void *run();
    bool writeToFile(Hit hit);
    void closeFile();

    pthread_mutex_t mutex;
    pthread_cond_t hitAvailable;

    queue<Hit> hits;
    ofstream ofs;

    string fileName;
};

#endif // DATAWRITER_H
