/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the graphics core library.

the graphics core library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

the graphics core library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the graphics core library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _CONTROLLER_HPP_
#define _CONTROLLER_HPP_

#include "curves.hpp"

enum TimelineMode {TIME_WRAP, TIME_CLAMP, TIME_FREE};
enum ControllerClass {CTRL_CURVE, CTRL_SIN, CTRL_COS};

enum {
	CTRL_X		= 1,	// 0001
	CTRL_Y		= 2,	// 0010
	CTRL_Z		= 4,	// 0100
	CTRL_XY		= 3,	// 0011
	CTRL_YZ		= 6,	// 0110
	CTRL_XZ 	= 5,	// 0101
	CTRL_XYZ	= 7		// 0111
};
	

class MotionController {
private:
	Curve *curve;
	float freq, ampl, phase;
	float (*freq_func)(float);
	float (*ampl_func)(float);
	unsigned long start_time, end_time;
	
	TimelineMode time_mode;
	ControllerClass ctrl_type;
	unsigned int axis_flags;
	
public:
	MotionController(ControllerClass ctype = CTRL_CURVE, TimelineMode mode = TIME_CLAMP);
	MotionController(Curve *curve, unsigned long start, unsigned long end, TimelineMode mode = TIME_CLAMP);
	
	void SetCurve(Curve *curve);
	void SetSinFunc(float freq, float ampl, float phase = 0.0f);
	void SetSinFunc(float (*freq_func)(float), float(*ampl_func)(float));
	
	void SetTiming(unsigned long start, unsigned long end);
	void SetTimelineMode(TimelineMode tmode);
	void SetControllerType(ControllerClass ctype);
	void SetControlAxis(unsigned int axis_flags);
	
	Curve *GetCurve();
	unsigned long GetStartTime() const;
	unsigned long GetEndTime() const;
	TimelineMode GetTimelineMode() const;
	unsigned int GetControlAxis() const;
	
	Vector3 operator ()(unsigned long time) const;
};
	

#endif	// _CONTROLLER_HPP_
