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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include "texman.hpp"
#include "hashtable.hpp"
extern "C" {
#include "image.h"
}
using std::string;

static HashTable<string, Texture*> textures;
static bool texman_initialized = false;

/*
 * Hashing algorithm for strings from:
 * Sedgewick's "Algorithms in C++, third edition" 
 * parts 1-4, Chapter 14 (hashing) p.593
 */
template <class KeyType>
static unsigned int Hash(const KeyType &key, unsigned long size) {
	int hash = 0, a = 31415, b = 27183;
	char *str = strdup(((string)key).c_str());
	char *sptr = str;

	while(*sptr) {
		hash = (a * hash + *sptr++) % size;
		a = a * b % (size - 1);
	}

	free(str);

	return (unsigned int)(hash < 0 ? (hash + size) : hash);
}



static void InitTexMan() {
	textures.SetHashFunction(Hash);
	texman_initialized = true;
}

void AddTexture(Texture *texture, const char *fname) {

	if(!texman_initialized) InitTexMan();

	if(!fname) {	// enter a randomly named texture
		textures.Insert(tmpnam(0), texture);
	} else {
		textures.Insert(fname, texture);
	}
}

// TODO: implement this one, after making HashTable remove by value
void RemoveTexture(Texture *texture) {
}

Texture *FindTexture(const char *fname) {

	if(!texman_initialized) InitTexMan();

	Pair<string, Texture*> *res = textures.Find(fname);
	return res ? res->val : 0;
}


/* ----- GetTexture() function -----
 * first looks in the texture database in constant time (hash table)
 * if the texture is already there it just returns the pointer. If the
 * texture is not there it tries to load the image data, create the texture
 * and return it, and if it fails it returns a NULL pointer
 */
Texture *GetTexture(const char *fname) {

	Texture *tex;
	if((tex = FindTexture(fname))) return tex;

	PixelBuffer pbuf;
	if(!(pbuf.buffer = (Pixel*)LoadImage(fname, &pbuf.width, &pbuf.height))) {
		return 0;
	}

	tex = new Texture;
	tex->SetPixelData(pbuf);
	return tex;
}
