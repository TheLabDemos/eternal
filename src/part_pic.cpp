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
#include "part_pic.hpp"

static const int part_count = 10;
static const int path_count = 5;
static Texture *pic, *psys;
static Vertex *vorig;
static Curve *curve;
static CatmullRomSpline ppath[path_count];
static float path_offset[path_count][part_count];

static const unsigned long fadein_dur = 1000;
static const unsigned long start_rblur = 7000;
static const unsigned long end_rblur = 11000;
static const unsigned long start_dist = 10000;
static const unsigned long start_zoomout = 14000;
static const unsigned long end_zoomout = 23000;
static const unsigned long start_psys = 21000;
static const unsigned long start_wave_fadeout = 20000;
static const unsigned long end_wave_fadeout = 23000;
static const unsigned long start_logo_fadein = 22000;
static const unsigned long end_logo_fadein = 24000;
static const unsigned long start_trans = 27000;
static const unsigned long end_trans = 30000;


static void DrawParticles(int i, unsigned long time); 

PartPic::PartPic() {
	SetName("part_pic");

	light.SetPosition(Vector3(0, 50, -80));
	cam = TargetCamera(Vector3(0, 0, 0), Vector3(0, 0, 0));

	plane = new Object;
	CreatePlane(plane->GetTriMeshPtr(), Plane(Vector3(0,0,0)), Vector2(32, 24), 50);

	plane->GetMaterialPtr()->SetTexture(GetTexture("data/apocalypse.png"), TEXTYPE_DIFFUSE);
	plane->SetBlending(true);

	int count = plane->GetTriMeshPtr()->GetVertexArray()->GetCount();
	vorig = new Vertex[count];
	memcpy(vorig, plane->GetTriMeshPtr()->GetVertexArray()->GetData(), count * sizeof(Vertex));

	curve = new CatmullRomSpline;
	curve->AddControlPoint(Vector3(0, 0, -26));
	curve->AddControlPoint(Vector3(-30, 10, -66));
	curve->AddControlPoint(Vector3(-40, 10, -106));
	curve->AddControlPoint(Vector3(0, 0, -116));
	curve->AddControlPoint(Vector3(30, -10, -96));
	curve->SetArcParametrization(true);
	
	cam.AddController(MotionController(curve, start_zoomout, end_zoomout), CTRL_TRANSLATION);

	for(int i=0; i<path_count; i++) {
		ppath[i].AddControlPoint(Vector3(1.3, frand(0.5) + 0.25, 0));
		ppath[i].AddControlPoint(Vector3(0.9, frand(0.5) + 0.25, 0));
		ppath[i].AddControlPoint(Vector3(0.4, frand(0.5) + 0.25, 0));
		ppath[i].AddControlPoint(Vector3(-0.2, frand(0.5) + 0.25, 0));
		ppath[i].SetArcParametrization(true);

		for(int j=0; j<part_count; j++) {
			path_offset[i][j] = frand(0.1f);
		}
	}

	psys = GetTexture("data/psys02.png");
}

PartPic::~PartPic() {
	delete plane;
	delete curve;
	delete [] vorig;
}

void PartPic::DrawPart() {
	float t = (float)time / 1000.0f;

	if(time >= end_zoomout - 100) {
		cam.SetPosition(Vector3(30, -10, -96));
		cam.Activate();
	} else {
		cam.Activate(time);
	}
	light.SetGLLight(0);

	if(time >= start_rblur && time < end_rblur) {
		SetRenderTarget(dsys::tex[dsys::RT_TEX0]);
	}

	if(time >= start_wave_fadeout && time < end_wave_fadeout) {
		float t = ((float)time / 1000.0f - (float)start_wave_fadeout / 1000.0f) / ((float)(end_wave_fadeout - start_wave_fadeout) / 1000.0f);
		plane->GetMaterialPtr()->alpha = 1.0f - t;
	}
	
	
	if(time >= start_dist) {
		Distort(time);
	}
	plane->Render();

	if(time >= start_rblur && time < end_rblur) {
		dsys::Overlay(0, Vector3(0,0), Vector3(1,1), Color(0.0f, 0.0f, 0.0f, 0.94f));
		SetRenderTarget(0);
		Clear(0);

		float ammount = (t - (float)start_rblur / 1000.0f) / ((float)(end_rblur - start_rblur) / 1000.0f);
		dsys::RadialBlur(dsys::tex[dsys::RT_TEX0], 2.0f * sin(ammount * pi), Vector3(0.25f + ammount / 2.0f, 0.5f), true);
	}
	
	if(time < fadein_dur) {
		float alpha = 1.0f - t / ((float)fadein_dur / 1000.0f);
		dsys::Overlay(0, Vector2(0,0), Vector2(1,1), Color(0.0f, 0.0f, 0.0f, alpha));
	}

	if(time >= start_logo_fadein) {
		float alpha = (t - (float)start_wave_fadeout / 1000.0f) / ((float)(end_logo_fadein - start_logo_fadein) / 1000.0f) - 1.0f;
		if(time >= end_logo_fadein) alpha = 1.0f;
		
		float shrink = 0.0f;
		Vector2 c1(0.0f, 0.35f), c2(1.0f, 0.70f);
		if(time >= start_trans && time < end_trans) {
			shrink = (t - (float)start_trans / 1000.0f) / ((float)(end_trans - start_trans) / 1000.0f) / 2.0f;
		}
		dsys::Overlay(GetTexture("data/eternal.png"), c1 + Vector2(shrink, shrink * 0.35), c2 - Vector2(shrink, shrink * 0.35), Color(1.0f, 1.0f, 1.0f, alpha));
	}
	for(int i=0; i<path_count; i++) {
		DrawParticles(i, time - start_logo_fadein);
	}

}

void PartPic::Distort(unsigned long time) {
	float t = (float)time / 1000.0f;
	Vertex *targ = plane->GetTriMeshPtr()->GetModVertexArray()->GetModData();
	int count = plane->GetTriMeshPtr()->GetVertexArray()->GetCount();

	for(int i=0; i<count; i++) {
		float dist = vorig[i].pos.LengthSq();
		float h = (2.5f * sin((dist/20.0f) - t * 3.0f));// * (dist > 0.0f ? (1.0f / (dist*0.5f)) : 1.0f);

		targ[i].pos = vorig[i].pos + Vector3(0, 0, h);
	}
	plane->GetTriMeshPtr()->CalculateNormals();
}

static const float part_size = 0.04f;
static void DrawParticles(int i, unsigned long time) {
	float t = (float)time / 5000.0f;
	
	SetAlphaBlending(true);
	for(int j=0; j<part_count; j++) {
		float x = t - (float)j * path_offset[i][j];
		if(x < 0.0f) x = 0.0f;
		if(x > 0.95f) x = 0.95f;
		Vector3 vec = ppath[i].Interpolate(x);
		Vector2 c1(vec.x-part_size, vec.y-part_size);
		Vector2 c2(vec.x+part_size, vec.y+part_size);

		SetBlendFunc(BLEND_ONE, BLEND_ONE);
		dsys::Overlay(psys, c1, c2, 1.0f, false);
	}
	SetAlphaBlending(false);
}
