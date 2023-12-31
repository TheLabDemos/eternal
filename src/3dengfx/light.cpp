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

#include "3denginefx.hpp"
#include "opengl.h"
#include "light.hpp"

Light::Light() {
	ambient_color = Color(0, 0, 0);
	diffuse_color = Color(1.0f, 1.0f, 1.0f);
	specular_color = Color(1.0f, 1.0f, 1.0f);
	intensity = 1.0f;
	attenuation[0] = 1.0f;
	attenuation[1] = 0.0f;
	attenuation[2] = 0.0f;
}

Light::~Light() {}

void Light::SetColor(const Color &c, unsigned short color_flags) {
	if(color_flags & LIGHTCOL_AMBIENT) {
		ambient_color = c;
	}
	
	if(color_flags & LIGHTCOL_DIFFUSE) {
		diffuse_color = c;
	}
	
	if(color_flags & LIGHTCOL_SPECULAR) {
		specular_color = c;
	}
}


void Light::SetColor(const Color &amb, const Color &diff, const Color &spec) {
	ambient_color = amb;
	diffuse_color = diff;
	specular_color = spec;
}

Color Light::GetColor(unsigned short which) const {
	switch(which) {
	case LIGHTCOL_AMBIENT:
		return ambient_color;
		
	case LIGHTCOL_SPECULAR:
		return specular_color;
		
	case LIGHTCOL_DIFFUSE:
	default:
		return diffuse_color;
	}
}

void Light::SetIntensity(scalar_t intensity) {
	this->intensity = intensity;
}

scalar_t Light::GetIntensity() const {
	return intensity;
}

void Light::SetAttenuation(scalar_t att0, scalar_t att1, scalar_t att2) {
	attenuation[0] = att0;
	attenuation[1] = att1;
	attenuation[2] = att2;
}

scalar_t Light::GetAttenuation(int which) const {
	return attenuation[which];
}

Vector3 Light::GetAttenuationVector() const {
	return Vector3(attenuation[0], attenuation[1], attenuation[2]);
}


PointLight::PointLight(const Vector3 &pos, const Color &col) {
	SetPosition(pos);
	diffuse_color = specular_color = col;
}

PointLight::~PointLight() {}

void PointLight::SetGLLight(int n, unsigned long time) const {
	int light_num = GL_LIGHT0 + n;
	
	Vector4 pos;
	if(time == XFORM_LOCAL_PRS) {
		pos = (Vector4)local_prs.position;
	} else {
		pos = (Vector4)GetPRS(time).position;
	}
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	Matrix4x4 test = view_matrix;
	test.Translate(pos);
	LoadMatrixGL(test);

	float position[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float amb[] = {ambient_color.r, ambient_color.g, ambient_color.b, ambient_color.a};
	float diff[] = {diffuse_color.r, diffuse_color.g, diffuse_color.b, diffuse_color.a};
	float spec[] = {specular_color.r, specular_color.g, specular_color.b, specular_color.a};
	
	glLightfv(light_num, GL_POSITION, position);
	glLightfv(light_num, GL_AMBIENT, amb);
	glLightfv(light_num, GL_DIFFUSE, diff);
	glLightfv(light_num, GL_SPECULAR, spec);
	glLightf(light_num, GL_CONSTANT_ATTENUATION, (float)attenuation[0]);
	glLightf(light_num, GL_LINEAR_ATTENUATION, (float)attenuation[1]);
	glLightf(light_num, GL_QUADRATIC_ATTENUATION, (float)attenuation[2]);
	
	glEnable(light_num);
	
	glPopMatrix();
}
