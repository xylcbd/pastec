#ifndef INDEX_H
#define INDEX_H

#include <list>

using namespace std;

class HitForward;

class Index
{
    virtual u_int32_t addImage(unsigned i_imageId, list<HitForward> hitList) = 0;
    virtual u_int32_t removeImage(const unsigned i_imageId) = 0;
};

#endif // INDEX_H
