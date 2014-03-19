#ifndef DATAWRITER_H
#define DATAWRITER_H

#include <iostream>
#include <sys/types.h>
#include <fstream>
#include <queue>

#include "thread.h"

using namespace std;


struct HitForward
{
    u_int32_t i_wordId;
    u_int32_t i_imageId;
    u_int16_t i_angle;
    u_int16_t x;
    u_int16_t y;
};


class ForwardIndexBuilder
{
public:
    ForwardIndexBuilder(string fileName);
    bool build(unsigned i_nbImages, char *p_imageIds);

private:
    bool addImage(unsigned i_imageId);

    ofstream ofs;

    string fileName;
};

#endif // DATAWRITER_H
