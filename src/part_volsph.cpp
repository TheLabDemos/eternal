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
#include "part_volsph.hpp"

static const int vol_tex_count = 7;
static Texture *vol_tex, *credits[5];

static float ypos[5];

static void Credits(unsigned long time);

PartVolSph::PartVolSph() {
	SetName("volsph");
	
	// setup the scene
	light.SetPosition(Vector3(10, 10, -20));
	cam.SetPosition(Vector3(0, 0, -6));

	
	SetDataPath("data/", DPATH_GEOM);
	if(!(vol_sph = LoadObject("SphereLow", "spheres.ase", LGEOM_FILE))) {
		std::cerr << "failed to load object SphereLow from spheres.ase\n";
		return;
	}

	if(!(sph = LoadObject("SphereMed", "spheres.ase", LGEOM_FILE))) {
		std::cerr << "failed to load object SphereMed from spheres.ase\n";
		return;
	}

	if(!(vol_tex = GetTexture("data/vol/lava02.png"))) {
		std::cerr << "failed to load data/vol/lava02.png\n";
		return;
	}

	if(!(tex = GetTexture("data/lavacr_s.png"))) {
		std::cerr << "failed to load data/lavacr_s.png\n";
		return;
	}

	for(int i=0; i<5; i++) {
		char fname[] = "data/credits/credit#.png";
		*strchr(fname, '#') = '0' + i;
		if(!(credits[i] = GetTexture(fname))) {
			std::cerr << "could not load: " << fname << std::endl;
		}
	}		

	sph->GetMaterialPtr()->SetTexture(tex, TEXTYPE_DIFFUSE);
	Material *mat = vol_sph->GetMaterialPtr();
	mat->SetTexture(vol_tex, TEXTYPE_DIFFUSE);
	mat->emissive_color = Color(1.0f, 1.0f, 1.0f);
	mat->ambient_color = mat->diffuse_color = Color(0,0,0);

	vol_sph->SetBlendingMode(BLEND_ONE, BLEND_ONE);
	vol_sph->SetBlending(true);

	sph->SetDynamic(false);
	vol_sph->SetDynamic(false);

	ypos[0] = -0.25f;
	ypos[1] = 0.30f;
	ypos[2] = -0.13f;
	ypos[3] = 0.24f;
	ypos[4] = 0.0f;
}

PartVolSph::~PartVolSph() {
	delete vol_sph;
	delete sph;
}

static const unsigned int start_credits = 1000;
static const unsigned int start_fade = 15000;
static const unsigned int end_fade = 18000;

void PartVolSph::DrawPart() {
	cam.Activate();
	light.SetGLLight(0);

	int tindex = time / 1000;
	if(tindex >= vol_tex_count) tindex = vol_tex_count - 1;
	//vol_sph->GetMaterialPtr()->SetTexture(vol_tex[tindex], TEXTYPE_DIFFUSE);

	SetRenderTarget(dsys::tex[dsys::RT_TEX0]);
	
	float t = (float)time / 1000.0f;
	sph->SetRotation(Vector3(t, t*2, 0));
	vol_sph->SetRotation(Vector3(t, t*2, 0));
	
	SetZBuffering(false);

	static const int vol_sph_count = 30;
	static const float vol_max_scale = 2.5f;
	static const float vol_scale_inc = 0.5f * vol_max_scale / (float)vol_sph_count;
	static const float vol_alpha_dec = 1.0f / (float)vol_sph_count;
	
	SetFrontFace(ORDER_CCW);
	float s = 1.0f;
	float alpha = 1.0f;
	Material *mptr = vol_sph->GetMaterialPtr();
	for(int i=0; i<vol_sph_count; i++) {
		mptr->emissive_color = Color(alpha, alpha, alpha);

		vol_sph->SetScaling(Vector3(s, s, s)); 
		vol_sph->Render();
		s += vol_scale_inc;
		alpha -= vol_alpha_dec;
	}
	
	SetFrontFace(ORDER_CW);
	sph->Render();
	
	SetRenderTarget(dsys::tex[dsys::RT_TEX0]);
	s = 1.0f;
	alpha = 1.0f;
	for(int i=0; i<vol_sph_count; i++) {
		mptr->emissive_color = Color(alpha, alpha, alpha);

		vol_sph->SetScaling(Vector3(s, s, s)); 
		vol_sph->Render();
		s += vol_scale_inc;
		alpha -= vol_alpha_dec;
	}
	SetZBuffering(true);

	SetRenderTarget(0);

	// also use a slight radial blur to cover up the imperfections
	dsys::RadialBlur(dsys::tex[dsys::RT_TEX0], 0.2f);

	if(time >= start_credits) {
		Credits(time - start_credits);
	}

	/*
	if(time >= start_fade && time < end_fade) {
		float a = ((float)time / 1000.0f - (float)start_fade / 1000.0f) / ((float)(end_fade - start_fade) / 1000.0f);
		a = a * (float)(end_fade - start_fade) / 1000.0f / (float)(end_fade - start_fade) / 1000.0f;
		dsys::Overlay(0, Vector2(0,0), Vector2(1,1), Color(0.0f, 0.0f, 0.0f, a));		
	}*/
}

static const float start_interval = 2.5;

static void Credits(unsigned long time) {
	float t = (float)time / 1000.0f;

	SetAlphaBlending(true);
	SetBlendFunc(BLEND_SRC_ALPHA, BLEND_ONE);
	for(int i=0; i<5; i++) {
		float xoffs = 1.3 - t / 1.2f + (float)i * start_interval;
		float x = xoffs < 0.0f ? 0.0f : (xoffs > 1.0f ? 1.0f : xoffs);
		float alpha = cos(x * pi);

		xoffs /= 10.0f;

		Vector2 c0(0.3f, 0.4f), c1(0.8f, 0.6f);
		dsys::Overlay(credits[i], c0 + Vector2(xoffs, ypos[i]), c1 + Vector2(xoffs, ypos[i]), Color(0.0f, 0.0f, 0.0f, alpha), false);
	}
	SetAlphaBlending(false);
}
