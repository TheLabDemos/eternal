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
#include <vector>
#include <n3dmath2.hpp>
#include <SDL.h>
#include "3dengfx.hpp"
#include "dsys.hpp"
#include "sdlvf.h"

// parts
#include "part_start.hpp"
#include "part_volsph.hpp"
#include "part_hairy.hpp"
#include "part_tunnel.hpp"
#include "part_statues.hpp"
#include "part_tunnel2.hpp"
#include "part_pic.hpp"

using namespace std;

int Init();
void CleanUp();
bool UpdateGraphics();
int EventHandler(SDL_Event &event);

// ----- globals ------
std::vector<dsys::Part*> parts;

int main(int argc, char **argv) {
	
	if(Init() == -1) return -1;

	bool done = false;
	while(!done) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(EventHandler(event) == -1) done = true;
		}
		if(!UpdateGraphics()) {
			done = true;
		}
	}

	CleanUp();
}

int Init() {

	try {
		GraphicsInitParameters gip = LoadGraphicsContextConfig("3dengfx.conf");
		CreateGraphicsContext(gip);
	}
	catch(EngineException except) {
		cerr << except.GetMessage() << endl;
		return -1;
	}
	SDL_WM_SetCaption("The Lab Demos", 0);
	dsys::Init();

	Clear(0);
	dsys::Overlay(GetTexture("data/loading.png"), Vector2(0,0), Vector2(1,1), 1.0f);
	Flip();

	parts.push_back(new PartVolSph);
	parts.push_back(new PartStart);
	parts.push_back(new PartHairy);
	parts.push_back(new PartTunnel);
	parts.push_back(new PartStatues);
	parts.push_back(new PartTunnel2);
	parts.push_back(new PartPic);

	for(int i=0; i<(int)parts.size(); i++) {
		AddPart(parts[i]);
	}

	dsys::StartDemo();

	if(sdlvf_init("data/amigo-eternal.ogg") != SDLVF_PLAYING) {
		std::cerr << "could not open music\n";
	}
	sdlvf_seek(19.5);

	return 0;
}

void CleanUp() {
	for(int i=0; i<(int)parts.size(); i++) {
		delete parts[i];
	}
	dsys::CleanUp();
	DestroyGraphicsContext();
}

bool UpdateGraphics() {
	sdlvf_check();
	return dsys::UpdateGraphics() != -1;
}

// ---------- Event Handler --------------

int EventHandler(SDL_Event &event) {

	switch(event.type) {
	case SDL_QUIT:
		return -1;

	case SDL_KEYDOWN:
		if(event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == 'q') {
			return -1;
		}
		break;
	}

	return 0;
}
