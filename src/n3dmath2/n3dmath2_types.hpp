/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the n3dmath2 library.

The n3dmath2 library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The n3dmath2 library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the n3dmath2 library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _N3DMATH2_TYPES_HPP_
#define _N3DMATH2_TYPES_HPP_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif	// HAVE_CONFIG_H

#ifdef SINGLE_PRECISION_MATH
typedef float scalar_t;
#else
typedef double scalar_t;
#endif	// SINGLE_PRECISION_MATH

// -- class forward declarations --
class Vector2;
class Vector2i;
class Vector3;
class Vector4;
class Matrix3x3;
class Matrix4x4;
class Quaternion;
class Base;
class Ray;

#endif	// _N3DMATH2_TYPES_HPP_
