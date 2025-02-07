#ifndef Magnum_Primitives_Crosshair_h
#define Magnum_Primitives_Crosshair_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Function @ref Magnum::Primitives::crosshair2D(), @ref Magnum::Primitives::crosshair3D()
 */

#include "Magnum/Primitives/visibility.h"
#include "Magnum/Trade/Trade.h"

namespace Magnum { namespace Primitives {

/**
@brief 2D crosshair

2x2 crosshair (two crossed lines), centered at origin. Non-indexed
@ref MeshPrimitive::Lines with @ref VertexFormat::Vector2 positions. The
returned instance references @ref Trade::DataFlag::Global data --- pass the
mesh through @ref MeshTools::copy() to get a mutable copy, if needed.

@image html primitives-crosshair2d.png width=256px

@see @ref crosshair3D(), @ref axis2D(), @ref line2D()
*/
MAGNUM_PRIMITIVES_EXPORT Trade::MeshData crosshair2D();

/**
@brief 3D crosshair

2x2x2 crosshair (three crossed lines), centered at origin. Non-indexed
@ref MeshPrimitive::Lines with @ref VertexFormat::Vector3 positions. The
returned instance references @ref Trade::DataFlag::Global data --- pass the
mesh through @ref MeshTools::copy() to get a mutable copy, if needed.

@image html primitives-crosshair3d.png width=256px

@see @ref crosshair2D(), @ref axis2D(), @ref line3D()
*/
MAGNUM_PRIMITIVES_EXPORT Trade::MeshData crosshair3D();

}}

#endif
