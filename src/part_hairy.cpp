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

#include <iostream>
#include "part_hairy.hpp"

inline static float usin(float x) {
	return (1.0f + sin(x)) / 2.0f;
}

inline static float ucos(float x) {
	return (1.0f + cos(x)) / 2.0f;
}

static Object *quad;
static Texture *back, *greets;

static const unsigned long greets_start = 5000;
static const unsigned long music_fade_start = 5000;//22000;
static const unsigned long music_fade_end = 8000;//25000;

PartHairy::PartHairy() {
	SetName("part_hairy");

	light.SetPosition(Vector3(0, 10, -20));
	cam.SetPosition(Vector3(0, 0, -10));

	SetDataPath("data/", DPATH_GEOM);
	if(!(sph = LoadObject("SphereMed", "spheres.ase", LGEOM_FILE))) {
		fprintf(stderr, "failed to load object \"SphereMed\" from \"spheres.ase\"\n");
		return;
	}

	const VertexArray *sph_varray = sph->GetTriMeshPtr()->GetVertexArray();
	orig_verts = new Vertex[sph_varray->GetCount()];
	memcpy(orig_verts, sph_varray->GetData(), sph_varray->GetCount() * sizeof(Vertex));
		
	hair_tex = GetTexture("data/fur.png");
	back = GetTexture("data/greetz-background.png");
	greets = GetTexture("data/full-greetz-without-background.png");

	quad = new Object;
	CreatePlane(quad->GetTriMeshPtr(), Plane(Vector3(0,0,0)), Vector2(9, 9), 1);
	quad->GetMaterialPtr()->SetTexture(back, TEXTYPE_DIFFUSE);
	quad->SetZWrite(false);
}

PartHairy::~PartHairy() {
	delete sph;
}

static const unsigned long speed_time = 3000;

void PartHairy::DrawPart() {
	cam.Activate();
	light.SetGLLight(0);

	float t = (float)time / 1000.0f;


	if(time < speed_time) {
		Deform(usin(t/2.0f), 1.0f);
	} else {
		Deform(usin(t*10.0f) * 0.5f + 0.8f, 14.0f);
	}
	//sph->SetRotation(Vector3(sin(t/10.0f)*1.5f, sin(t/10.0f), 0));
	//sph->SetPosition(Vector3(0, 0, 5.0f * usin(t*10.0f)));
	sph->SetRotation(Vector3(t, t, 0));

	SetRenderTarget(dsys::tex[dsys::RT_TEX0]);

	quad->SetRotation(Vector3(0, 0, t));
	quad->Render();

	sph->Render();
	SetZBuffering(false);
	sph->SetBlending(true);
	sph->GetMaterialPtr()->SetTexture(hair_tex, TEXTYPE_DIFFUSE);

	const int sph_count = 10;
	const float max_scale = 1.2f;
	const float scale_inc = (max_scale - 1.0f) / (float)sph_count;
	float scale = 1.0f;
	for(int i=0; i<sph_count; i++) {
		sph->GetMaterialPtr()->alpha = 1.0f - ((float)i / (float)sph_count);
		sph->SetScaling(Vector3(scale, scale, scale));
		sph->Render();
		scale += scale_inc;
	}
	sph->SetScaling(Vector3(1, 1, 1));
	sph->GetMaterialPtr()->SetTexture(0, TEXTYPE_DIFFUSE);
	sph->SetBlending(false);
	SetZBuffering(true);

	SetRenderTarget(0);
	Clear(0);
	ClearZBufferStencil(1.0f, 0);

	dsys::RadialBlur(dsys::tex[dsys::RT_TEX0], 0.2f);
	
	if(time >= greets_start) {
		SetAlphaBlending(true);
		SetBlendFunc(BLEND_ONE, BLEND_ONE);
		dsys::Overlay(greets, Vector2(0,0), Vector2(1,1), 0.0f, false);
		SetAlphaBlending(false);
	}
}


void PartHairy::Deform(float intensity, float speed) {
	VertexArray *varray = sph->GetTriMeshPtr()->GetModVertexArray();
	Vertex *verts = varray->GetModData();
	int count = varray->GetCount();

	float t = speed * (float)time / 1000.0f;	// 80?

	static const Vector3 k(0, 0, 1);
	static const Vector3 j(0, 1, 0);

	for(int i=0; i<count; i++) {
		float sfact = 1.0f;
		Vector3 pos = orig_verts[i].pos;
		
		float angle_k = acos(DotProduct(k, pos));
		float angle_j = acos(DotProduct(j, pos));

		sfact += usin(angle_k * 3.0f) + ucos(t * angle_j);
		sfact += ucos(angle_j * 4.0f) + usin(t * angle_k);
		sfact *= intensity;
		
		verts[i].pos = pos + (pos * sfact / 3.0f);
	}
}
