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
#include "base_cam.hpp"

BaseCamera::BaseCamera(const Vector3 &trans, const Quaternion &rot) {
	SetPosition(trans);
	SetRotation(rot);
	up = Vector3(0, 1, 0);
	fov = quarter_pi;
	near = 1.0;
	far = 1000.0;
}

BaseCamera::~BaseCamera() {}

void BaseCamera::SetUpVector(const Vector3 &up) {
	this->up = up;
}

void BaseCamera::SetFOV(scalar_t angle) {
	fov = angle;
}

scalar_t BaseCamera::GetFOV() const {
	return fov;
}

void BaseCamera::SetClippingPlanes(scalar_t near, scalar_t far) {
	this->near = near;
	this->far = far;
}

void BaseCamera::SetClippingPlane(scalar_t val, ClipPlane which) {
	if(which == CLIP_NEAR) {
		near = val;
	} else {
		far = val;
	}
}

scalar_t BaseCamera::GetClippingPlane(ClipPlane which) const {
	return which == CLIP_NEAR ? near : far;
}


void BaseCamera::Zoom(scalar_t zoom_factor, unsigned long msec) {
	Vector3 zoom_dir(0, 0, zoom_factor);
	PRS prs = GetPRS(msec);

	zoom_dir.Transform(prs.rotation.Inverse());
	Translate(zoom_dir, msec);
}

void BaseCamera::Pan(const Vector2 &dir, unsigned long msec) {
	Vector3 i(1, 0, 0), j(0, 1, 0);
	
	PRS prs = GetPRS(msec);
	
	i.Transform(prs.rotation.Inverse());
	j.Transform(prs.rotation.Inverse());
	
	Translate(i * dir.x);
	Translate(j * dir.y);
}

void BaseCamera::Roll(scalar_t angle, unsigned long msec) {
	Vector3 dir(0, 0, 1);
	dir.Transform(GetPRS(msec).rotation);

	Quaternion q(dir, angle);
	up = Vector3(0, 1, 0);
	up.Transform(q);
}
