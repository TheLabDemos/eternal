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
#include "part_start.hpp"

static Object *land;
static Vertex *vorig;
static Texture *grid;

PartStart::PartStart() {
	SetName("part_start");

	light[0].SetPosition(Vector3(100, 100, -100));
	light[1].SetPosition(Vector3(-100, 30, -80));
	cam = TargetCamera(Vector3(0, 20, 0), Vector3(0, 0, 0));

	SetDataPath("data/", DPATH_GEOM);
	SetDataPath("data/", DPATH_TEX);

	if(!(thelab = LoadObject("thelab", "thelab.ase", LGEOM_FILE))) {
		std::cerr << "failed to load object \"thelab\" from \"thelab.ase\"\n";
		return;
	}

	if(!(nuclear = LoadObject("nuclear", "thelab.ase", LGEOM_FILE))) {
		std::cerr << "failed to load object \"nuclear\" from \"thelab.ase\"\n";
		return;
	}

	if(!(raw = LoadObject("rawnoise", "thelab.ase", LGEOM_FILE))) {
		std::cerr << "failed to load object \"rawnoise\" from \"thelab.ase\"\n";
		return;
	}

	if(!(amigo = LoadObject("amigo", "thelab.ase", LGEOM_FILE))) {
		std::cerr << "failed to load object \"amigo\" from \"thelab.ase\"\n";
		return;
	}
	
	thelab->SetDynamic(false);
	nuclear->SetDynamic(false);
	raw->SetDynamic(false);
	amigo->SetDynamic(false);

	thelab->GetMaterialPtr()->alpha = 0.0f;
	thelab->SetBlending(true);

	// assign controllers to the members logos
	MotionController xctrl(CTRL_SIN, TIME_FREE);
	xctrl.SetControlAxis(CTRL_X);
	MotionController zctrl(CTRL_COS, TIME_FREE);
	zctrl.SetControlAxis(CTRL_Z);
	
	// nuclear
	xctrl.SetSinFunc(1.0f, 30.0f);
	zctrl.SetSinFunc(1.0f, 30.0f);
	nuclear->AddController(xctrl, CTRL_TRANSLATION);
	nuclear->AddController(zctrl, CTRL_TRANSLATION);

	// rawnoise
	xctrl.SetSinFunc(1.0f, 30.0f, two_pi / 3.0f);
	zctrl.SetSinFunc(1.0f, 30.0f, two_pi / 3.0f);
	raw->AddController(xctrl, CTRL_TRANSLATION);
	raw->AddController(zctrl, CTRL_TRANSLATION);

	// amigo
	xctrl.SetSinFunc(1.0f, 30.0f, 2.0f * two_pi / 3.0f);
	zctrl.SetSinFunc(1.0f, 30.0f, 2.0f * two_pi / 3.0f);
	amigo->AddController(xctrl, CTRL_TRANSLATION);
	amigo->AddController(zctrl, CTRL_TRANSLATION);

	// assign controller to the camera
	xctrl.SetSinFunc(0.5f, 70.0f);
	zctrl.SetSinFunc(0.5f, 70.0f);
	cam.AddController(xctrl, CTRL_TRANSLATION);
	cam.AddController(zctrl, CTRL_TRANSLATION);


	const int subdiv = 20;
	
	land = new Object();
	CreatePlane(land->GetTriMeshPtr(), Plane(Vector3(0,0,0)), Vector2(80, 80), subdiv);
	land->Rotate(Vector3(-half_pi, 0, 0));
	//land->SetWireframe(true);
	
	const int size = 128;
	const int grid_count = subdiv+1;
	grid = new Texture(size, size);
	grid->Lock();
	for(int i=0; i<size; i++) {
		for(int j=0; j<size; j++) {
			grid->buffer[j * size + i] = (i % (size / grid_count)) && (j % (size / grid_count)) ? 0xff000000 : 0xffffffff;
		}
	}
	grid->Unlock();
	land->GetMaterialPtr()->SetTexture(grid, TEXTYPE_DIFFUSE);
	land->SetZWrite(false);
	land->SetBlending(true);
	
	int count = land->GetTriMeshPtr()->GetVertexArray()->GetCount();
	vorig = new Vertex[count * sizeof(Vertex)];
	memcpy(vorig, land->GetTriMeshPtr()->GetVertexArray()->GetData(), count * sizeof(Vertex));
}

PartStart::~PartStart() {
	delete thelab;
	delete nuclear;
	delete raw;
	delete amigo;
	delete land;
	delete vorig;
}

static const unsigned long start_fade = 2000;
static const unsigned long end_fade = 6000;

static const unsigned long start_rblur = 7000;
static const unsigned long end_rblur = 11000;

static const unsigned long nuclear_time = 12000;
static const unsigned long raw_time = 14000;
static const unsigned long amigo_time = 16000;

static const unsigned long start_rblur2 = 18000;
static const unsigned long end_rblur2 = 21000;

void PartStart::DrawPart() {
	cam.Activate(time);
	for(int i=0; i<2; i++) {
		light[i].SetGLLight(i);
	}

	float t = (float)time / 1000.0f;

	if(time >= start_fade && time < end_fade) {
		thelab->GetMaterialPtr()->alpha = (t - (float)start_fade/1000.0f)/((float)end_fade/1000.0f);
	}
	if(	(time >= start_rblur && time < end_rblur) ||
		(time >= start_rblur2 && time < end_rblur2)) {
		SetRenderTarget(dsys::tex[dsys::RT_TEX0]);
	}
	
	land->SetPosition(Vector3(0, -10.0f, 0));
	DeformLand();
	SetZBuffering(false);
	land->Render();
	SetZBuffering(true);

	thelab->SetScaling(Vector3(3.0f, 3.0f, 3.0f));

	nuclear->SetRotation(Vector3(0, t, 0));
	raw->SetRotation(Vector3(0, t*1.5f, 0));
	amigo->SetRotation(Vector3(0, t/1.5f, 0));	

	SetAutoNormalize(true);
	thelab->Render();
	SetAutoNormalize(false);
	
	if(time >= nuclear_time) nuclear->Render(time);
	if(time >= raw_time) raw->Render(time);
	if(time >= amigo_time) amigo->Render(time);

	
	if(time >= start_rblur && time < end_rblur) {
		SetRenderTarget(0);
		Clear(0);

		float blur = (t - (float)start_rblur / 1000.0f) / ((float)(end_rblur - start_rblur) / 1000.0f);
		//dsys::RadialBlur(dsys::tex[dsys::RT_TEX0], 2.0f * sin(blur * pi), Vector3(0.25f + blur / 2.0f, 0.5f), true);
		dsys::RadialBlur(dsys::tex[dsys::RT_TEX0], 2.0f * sin(blur * pi));
		//dsys::DirBlur(dsys::tex[dsys::RT_TEX0], sin(blur * pi), dsys::BLUR_DIR_X);
	}

	if(time >= start_rblur2 && time < end_rblur2) {
		SetRenderTarget(0);
		Clear(0);

		float blur = (t - (float)start_rblur2 / 1000.0f) / ((float)(end_rblur2 - start_rblur2) / 1000.0f);
		dsys::RadialBlur(dsys::tex[dsys::RT_TEX0], 2.0f * blur);
		//dsys::Negative();
	}

	//dsys::Flash(time, 1000, 1000);
	//dsys::Flash(time, nuclear_time, 500);
	//dsys::Flash(time, raw_time, 500);
	//dsys::Flash(time, amigo_time, 500);
}

void PartStart::DeformLand() {
	float t = (float)time / 500.0f;
	int count = land->GetTriMeshPtr()->GetVertexArray()->GetCount();
	Vertex *targ =  land->GetTriMeshPtr()->GetModVertexArray()->GetModData();

	for(int i=0; i<count; i++) {
		float x = vorig[i].pos.x / 10.0f;
		float y = vorig[i].pos.y / 10.0f;

		//float offs = sin(x * cos(t/10.0f) * 0.3) + cos(y * sin(t/10.0f) * 0.3);
		float offs = cos(x + t) + sin(y + t);
		targ[i].pos = vorig[i].pos + Vector3(0, 0, offs * 4.0f);
	}
	land->GetTriMeshPtr()->CalculateNormals();
}
