/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "camera.hpp"

void Camera::Activate(unsigned long msec) const {
	extern Matrix4x4 view_matrix;
	
	PRS prs = GetPRS(msec);

	view_matrix = prs.rotation.Inverse().GetRotationMatrix();
	view_matrix.Translate(-prs.position);
}


TargetCamera::TargetCamera(const Vector3 &trans, const Vector3 &target) {
	SetPosition(trans);
	fov = quarter_pi;
	near = 1.0;
	far = 1000.0;

	SetTarget(target);
}

TargetCamera::~TargetCamera() {}

void TargetCamera::SetTarget(const Vector3 &target) {
	this->target.SetPosition(target);
}

Vector3 TargetCamera::GetTarget(unsigned long msec) const {
	return target.GetPRS(msec).position;
}

void TargetCamera::Activate(unsigned long msec) const {
	extern Matrix4x4 view_matrix;

	Vector3 pos = GetPRS(msec).position;
	Vector3 targ = target.GetPRS(msec).position;

	Vector3 n = (targ - pos).Normalized();
	Vector3 u = CrossProduct(up, n).Normalized();
	Vector3 v = CrossProduct(n, u);

	float tx = -DotProduct(u, pos);
	float ty = -DotProduct(v, pos);
	float tz = -DotProduct(n, pos);

	view_matrix = Matrix4x4(u.x, u.y, u.z, tx,
							v.x, v.y, v.z, ty,
							n.x, n.y, n.z, tz,
							0.0, 0.0, 0.0, 1.0);
}

void TargetCamera::Roll(scalar_t angle, unsigned long msec) {
	Vector3 axis = target.GetPRS(msec).position - GetPRS(msec).position;
	Quaternion q(axis.Normalized(), fmod(angle, two_pi));
	up = Vector3(0, 1, 0);
	up.Transform(q);
}
