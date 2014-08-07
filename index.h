#ifndef INDEX_H
#define INDEX_H

#include <sys/types.h>
#include <list>
#include <string>

using namespace std;

class HitForward;

class Index
{
public:
    virtual u_int32_t removeImage(const unsigned i_imageId) = 0;
    virtual u_int32_t write(string backwardIndexPath) = 0;
    virtual u_int32_t clear() = 0;
    virtual u_int32_t load(string backwardIndexPath) = 0;
};

#endif // INDEX_H
