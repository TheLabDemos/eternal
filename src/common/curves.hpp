/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the eternal demo.

The eternal library is free software; you can redistribute it and/or modify
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
#ifndef _CURVES_HPP_
#define _CURVES_HPP_

#include <string>
#include "n3dmath2.hpp"
#include "linkedlist.hpp"	// FIXME (please make this work with std::list)


class Curve {
protected:
	LinkedList<Vector3> ControlPoints;
	Vector2 *Samples;	// used for parametrizing by arc length
	int SampleCount;
	bool ArcParametrize;

	Curve *ease_curve;	// ease in/out curve (1D, x&z discarded)
	int ease_sample_count, ease_step;
	
	void SampleArcLengths();
	float Parametrize(float t);
	float Ease(float t);

public:
	std::string name;

	Curve();
	virtual ~Curve();
	virtual void AddControlPoint(const Vector3 &cp);

	virtual int GetSegmentCount() const = 0;
	virtual void SetArcParametrization(bool state);
	virtual void SetEaseCurve(Curve *curve);
	virtual void SetEaseSampleCount(int count);

	virtual Vector3 Interpolate(float t) = 0;
};

class BSpline : public Curve {
public:
	virtual int GetSegmentCount() const;
	virtual Vector3 Interpolate(float t);	
};

class CatmullRomSpline : public Curve {
public:
	virtual int GetSegmentCount() const;
	virtual Vector3 Interpolate(float t);
};

#endif	// _CURVES_HPP_
