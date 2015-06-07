/*****************************************************************************
 * Copyright (C) 2014 Visualink
 *
 * Authors: Adrien Maglo <adrien@visualink.io>
 *
 * This file is part of Pastec.
 *
 * Pastec is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pastec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Pastec.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef PASTEC_HIT_H
#define PASTEC_HIT_H

#include <sys/types.h>
#include <cstdint>

struct Hit
{
    uint32_t i_imageId;
    uint16_t i_angle;
    uint16_t x;
    uint16_t y;
};

struct HitForward
{
    uint32_t i_wordId;
    uint32_t i_imageId;
    uint16_t i_angle;
    uint16_t x;
    uint16_t y;
};

#endif // PASTEC_HIT_H
