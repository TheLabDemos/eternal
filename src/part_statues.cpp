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

#include <SDL.h>
#include "part_statues.hpp"
#include "sceneloader.hpp"

static TargetCamera *cam;

PartStatues::PartStatues() {
	SetName("part_statues");
	
	SceneLoader::SetDataPath("data/");
	if(!SceneLoader::LoadScene("data/statues.3ds", &scene)) {
		std::cerr << "could not load scene \"statues.3ds\"\n";
		return;
	}

	torus[0] = scene->GetObject("tknot1");
	torus[1] = scene->GetObject("tknot2");
	torus[0]->SetDynamic(true);
	torus[1]->SetDynamic(true);

	scene->RemoveObject(torus[0]);
	scene->RemoveObject(torus[1]);

	torusdef = new Object(*torus[0]);
	torusdef->SetDynamic(true);

	cam = (TargetCamera*)scene->GetActiveCamera();
	Curve *path = scene->GetCurve("cpath01");
	path->SetArcParametrization(true);
	cam->SetPosition(Vector3(0, 0, 0));
	cam->AddController(MotionController(path, 0, 32000), CTRL_TRANSLATION);

	sky[0] = scene->GetObject("stars1");
	sky[1] = scene->GetObject("stars2");
	scene->RemoveObject(sky[0]);
	scene->RemoveObject(sky[1]);
	
	for(int i=0; i<2; i++) {
		sky[i]->SetZWrite(false);
		sky[i]->SetBlending(true);
		sky[i]->SetBlendingMode(BLEND_ONE, BLEND_ONE);
	}

	scene->SetAmbientLight(0.2f);
	
	//scene->AddCamera(&cam2);
	//scene->SetActiveCamera(&cam2);

	//SDL_WM_GrabInput(SDL_GRAB_ON);
	//SDL_ShowCursor(0);
}

PartStatues::~PartStatues() {
	//SDL_WM_GrabInput(SDL_GRAB_OFF);
	//SDL_ShowCursor(1);
	delete scene;
	delete torusdef;
	delete torus[0];
	delete torus[1];
}

static void HandleMouse(bool);

static const unsigned long rblur_start[2] = {7000, 22500};
static const unsigned long rblur_end[2] = {11000, 28000};

void PartStatues::DrawPart() {
	bool rblur = false;
	static bool neg = false;
	float ammount;

	// switch radial blur...
	for(int i=0; i<2; i++) {
		if(time > rblur_start[i] && time < rblur_end[i]) {
			rblur = true;
			float t = (float)(time - rblur_start[i]) / (float)(rblur_end[i] - rblur_start[i]);
			ammount = sin(t * pi);

			if(i == 0 && t > 0.5f) neg = true;
			if(i == 1 && t > 0.5f) neg = false;
		}
	}
	
	if(rblur) {
		SetRenderTarget(dsys::tex[dsys::RT_TEX0]);
	}
	
	scene->Render(time);
	
	sky[0]->Render();
	sky[1]->Render();

	MorphTorus(time + 500, 2000);
	torusdef->SetPosition(Vector3(70, 30, 70));
	torusdef->Render();

	torusdef->SetPosition(Vector3(-70, 30, -70));
	torusdef->Render();


	MorphTorus(time + 900, 2100);
	torusdef->SetPosition(Vector3(-70, 30, 70));
	torusdef->Render();

	torusdef->SetPosition(Vector3(70, 30, -70));
	torusdef->Render();

	if(rblur) {
		SetRenderTarget(0);
		Clear(0);
		dsys::RadialBlur(dsys::tex[dsys::RT_TEX0], ammount);
		//std::cerr << "ammount = " << ammount << std::endl;
	}

	if(neg) dsys::Negative();
}

void PartStatues::MorphTorus(unsigned long time, unsigned long duration) {
	const Vertex *varray[2];
	Vertex *final;

	if(time % (duration * 2) < duration) {
		varray[0] = torus[0]->GetTriMeshPtr()->GetVertexArray()->GetData();
		varray[1] = torus[1]->GetTriMeshPtr()->GetVertexArray()->GetData();
	} else {
		varray[0] = torus[1]->GetTriMeshPtr()->GetVertexArray()->GetData();
		varray[1] = torus[0]->GetTriMeshPtr()->GetVertexArray()->GetData();
	}
	final = torusdef->GetTriMeshPtr()->GetModVertexArray()->GetModData();

	int count = torus[0]->GetTriMeshPtr()->GetVertexArray()->GetCount();

	float t = (float)(time % duration) / (float)duration;
	for(int i=0; i<count; i++) {
		Vector3 v0 = varray[0][i].pos, v1 = varray[1][i].pos;
		final[i].pos = v0 + (v1 - v0) * t;
		final[i].pos *= 0.6f;
	}
}

static void HandleMouse(bool draw_only) {
	int x, y;
	SDL_GetRelativeMouseState(&x, &y);
	
	cam->Translate(Vector3(0, 0, -y));
	cam->Translate(Vector3(x, 0, 0));
}
