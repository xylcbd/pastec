#ifndef INDEX_H
#define INDEX_H

#include <list>

using namespace std;

class HitForward;

class Index
{
public:
    virtual u_int32_t removeImage(const unsigned i_imageId) = 0;
    virtual bool write() = 0;
    virtual bool clear() = 0;
};

#endif // INDEX_H
