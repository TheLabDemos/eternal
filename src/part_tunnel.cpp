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
#include "part_tunnel.hpp"
#include "sceneloader.hpp"

static TargetCamera *cam;
static Texture *overlay;

PartTunnel::PartTunnel() {
	SetName("part_tunnel");
	
	SceneLoader::SetDataPath("data/");
	if(!SceneLoader::LoadScene("data/tunnel.3ds", &scene)) {
		std::cerr << "could not load scene \"tunnel.3ds\"\n";
		return;
	}

	scene->SetAmbientLight(Color(0.2f, 0.2f, 0.2f));

	cam = (TargetCamera*)scene->GetActiveCamera();
	cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	cam->SetTarget(Vector3(0.0f, 0.0f, 0.0f));
	
	// make the controller for the camera motion
	MotionController xctrl(CTRL_SIN, TIME_FREE);
	xctrl.SetControlAxis(CTRL_X);
	xctrl.SetSinFunc(1.0f, 50.0f);
	MotionController zctrl(CTRL_COS, TIME_FREE);
	zctrl.SetControlAxis(CTRL_Z);
	zctrl.SetSinFunc(1.0f, 50.0f);

	cam->AddController(xctrl, CTRL_TRANSLATION);
	cam->AddController(zctrl, CTRL_TRANSLATION);

	xctrl.SetSinFunc(1.0f, 50.0f, 0.80f);
	zctrl.SetSinFunc(1.0f, 50.0f, 0.80f);
	cam->target.AddController(xctrl, CTRL_TRANSLATION);
	cam->target.AddController(zctrl, CTRL_TRANSLATION);

	overlay = GetTexture("data/overlay2.png");
}

PartTunnel::~PartTunnel() {
	delete scene;
}

void PartTunnel::DrawPart() {
	float t = (float)time / 1000.0f;

	cam->Roll(cos(t*2.0f)/2.0f + sin(t) + cos(t/2.0f) * 2.0f, time);
	
	scene->Render(time);

	dsys::Overlay(overlay, Vector3(0,0), Vector3(1,1), 1.0f);
}
