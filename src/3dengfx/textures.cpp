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
#include <cassert>
#include "opengl.h"
#include "textures.hpp"

static PixelBuffer undef_pbuf;

static void GenUndefImage(int x, int y) {
	if((int)undef_pbuf.width != x && (int)undef_pbuf.height != y) {
		if(undef_pbuf.buffer) {
			delete [] undef_pbuf.buffer;
		}
		undef_pbuf.width = x;
		undef_pbuf.height = y;
		undef_pbuf.pitch = x * sizeof(Pixel);
		undef_pbuf.buffer = new Pixel[x * y];

		for(int i=0; i<y; i++) {
			memset(&undef_pbuf.buffer[i * x], (i/(y/8))%2 ? 0x00ff0000 : 0, x * sizeof(Pixel));
		}
	}
}

Texture::Texture(int x, int y) {
	width = x;
	height = y;
	
	if(x != -1 && y != -1) {
		GenUndefImage(x, y);
		AddFrame(undef_pbuf);
	}
}		
		

void Texture::AddFrame() {
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	frame_tex_id.push_back(tex_id);
}

void Texture::AddFrame(const PixelBuffer &pbuf) {
	AddFrame();
	SetPixelData(pbuf);
}

void Texture::SetActiveFrame(unsigned int frame) {
	assert(frame < frame_tex_id.size());
	
	active_frame = frame;
	tex_id = frame_tex_id[active_frame];
}

unsigned int Texture::GetActiveFrame() const {
	return active_frame;
}

void Texture::Lock() {
	buffer = new Pixel[width * height];
	
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
}

void Texture::Unlock() {
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	
	delete [] buffer;
}

void Texture::SetPixelData(const PixelBuffer &pbuf) {
	
	if(!frame_tex_id.size()) {
		AddFrame();
	}
		
	width = pbuf.width;
	height = pbuf.height;
	
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pbuf.buffer);
	/* NOTE: is the previous function asyncronous? Do I have to wait for it
	** before going on, to ensure texture data integrity? possible bug if so.
	*/
}
