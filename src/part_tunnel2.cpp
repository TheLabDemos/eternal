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

#include "part_tunnel2.hpp"
#include "sceneloader.hpp"

struct Zone {
	Quaternion q;
	Matrix3x3 rmat;
	float max_dist;
};

static const int zone_count = 20;
static Zone zone[zone_count];

static TargetCamera *cam;
static Object *tunnel, *thing;
static Vertex *vorig;
static Texture *overlay;

static void DoTheThing(unsigned long time);

PartTunnel2::PartTunnel2() {
	SetName("part_tunnel2");
	
	SceneLoader::SetDataPath("data/");
	if(!SceneLoader::LoadScene("data/tunnel2.3ds", &scene)) {
		std::cerr << "could not load scene \"tunnel2.3ds\"\n";
		return;
	}

	//scene->SetAmbientLight(0.2f);
	tunnel = scene->GetObject("Torus01");
	thing = scene->GetObject("thing");	
	cam = (TargetCamera*)scene->GetActiveCamera();

	thing->SetDynamic(true);

	scene->RemoveObject(tunnel);

	int count = thing->GetTriMeshPtr()->GetVertexArray()->GetCount();
	vorig = new Vertex[count];
	memcpy(vorig, thing->GetTriMeshPtr()->GetVertexArray()->GetData(), count * sizeof(Vertex));
	
	float zone_width = 8.0f / zone_count;
	for(int i=0; i<zone_count; i++) {
		zone[i].max_dist = (i+1) * zone_width;
	}

	overlay = GetTexture("data/overlay1.png");
}

PartTunnel2::~PartTunnel2() {
	delete [] vorig;
	delete scene;
}

static const unsigned long start_tunnel = 11000;//15000;
static const unsigned long flash_duration = 300;
static const unsigned long neg_start = 14000;
//static const unsigned long neg_start_shrink = 8000;
//static const unsigned long neg_end = 11000;

void PartTunnel2::DrawPart() {
	float t = (float)time / 1000.0f;
	
	thing->SetPosition(Vector3(-46.042f, t < 2.0f ? (t*10.0f) - 20.0f : 0.0f, 0.0f));

	DoTheThing(time);

	cam->Roll(0);
	scene->Render();
	
	cam->Roll(t);
	cam->Activate();
	if(time >= start_tunnel) {
		tunnel->SetRotation(Vector3(0, t, 0));
		tunnel->Render();
	}

	float over_x = time < 2000 ? sin(t / 2.0f * pi / 2.0f) : 1.0f;
	dsys::Overlay(overlay, Vector3(over_x - 1, 0), Vector3(over_x, 1), 1.0f);

	static const float neg_size = 0.3f;
	if(time >= neg_start && time < neg_start + 10000) {
		float t = (time - neg_start) / 1000.0f;
		float neg_x = t / 7.0f;
		
		dsys::Negative(Vector2(neg_x - neg_size, 0.3), Vector2(neg_x, 0.7));
	}
	
	dsys::Flash(time, start_tunnel, flash_duration);
}


static void DoTheThing(unsigned long time) {
	static unsigned long last_update;
	if(time - last_update < 33.3333) return;
	last_update = time;
	
	float t = (float)time / 1000.0f;

	int count = thing->GetTriMeshPtr()->GetVertexArray()->GetCount();
	Vertex *targ = thing->GetTriMeshPtr()->GetModVertexArray()->GetModData();

	//Vector3 axis(cos(t*1.5f)/1.5f, sin(t/1.5f)*1.5f, 0);
	Vector3 axis(1, 0, 1);
	static Quaternion axis_q;

	axis_q.Rotate(Vector3(0, 0, 1), 0.1f);

	axis.Transform(axis_q);
	
	axis.Normalize();
	float angle = sin(t)/10.0f + cos(t*2.0f)/20.0f;
	
	for(int i=zone_count-1; i>0; i--) {
		zone[i].q = zone[i-1].q;
		zone[i].rmat = zone[i].q.GetRotationMatrix();
	}
	zone[0].q.Rotate(axis, angle);
	zone[0].rmat = zone[0].q.GetRotationMatrix();

	for(int i=0; i<count; i++) {
		float dist = vorig[i].pos.Length();

		int in_zone = 0;
		while(in_zone < zone_count && dist > zone[in_zone].max_dist) in_zone++;
		if(in_zone >= zone_count) {
			std::cerr << "vertex out of any zone!\n";
			continue;
		}

		targ[i].pos = vorig[i].pos;
		targ[i].pos.Transform(zone[in_zone].rmat);
	}
}
