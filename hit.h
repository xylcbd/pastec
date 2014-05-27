#ifndef HIT_H
#define HIT_H

#include <sys/types.h>

struct Hit
{
    u_int32_t i_imageId;
    u_int16_t i_angle;
    u_int16_t x;
    u_int16_t y;
};

struct HitForward
{
    u_int32_t i_wordId;
    u_int32_t i_imageId;
    u_int16_t i_angle;
    u_int16_t x;
    u_int16_t y;
};

#endif // HIT_H
