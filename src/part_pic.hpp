/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the eternal demo.

The eternal demo is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The eternal demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the eternal demo; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _PART_PIC_HPP_
#define _PART_PIC_HPP_

#include "demosys.hpp"
#include "3dengfx.hpp"

class PartPic : public dsys::Part {
protected:
	Object *plane;
	PointLight light;
	TargetCamera cam;

	virtual void DrawPart();
	void Distort(unsigned long time);

public:
	PartPic();
	~PartPic();
};

#endif	// _PART_PIC_HPP_
