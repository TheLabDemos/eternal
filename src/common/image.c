/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This is a small image loading library.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <png.h>
#include "image.h"

#define FILE_SIG_BYTES	8

/* Local function prototypes */
static void *LoadPNG(FILE *fp, unsigned long *xsz, unsigned long *ysz);

/* implementation */

void *LoadImage(const char *fname, unsigned long *xsz, unsigned long *ysz) {
	FILE *file;
	unsigned char signature[FILE_SIG_BYTES];

	if(!(file = fopen(fname, "rb"))) {
		fprintf(stderr, "Image loading error: could not open file %s\n", fname);
		return 0;
	}

	fread(signature, 1, FILE_SIG_BYTES, file);

	if(png_sig_cmp(signature, 0, FILE_SIG_BYTES) == 0) {
		return LoadPNG(file, xsz, ysz);
	}

	return 0;
}

void FreeImage(void *img) {
	free(img);
}

void *LoadPNG(FILE *fp, unsigned long *xsz, unsigned long *ysz) {
	png_struct *png_ptr;
	png_info *info_ptr;
	uint32_t i, j, **lineptr, *pixels, width, height;
	int channel_bits, color_type, ilace_type, compression, filtering;

	if(!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0))) {
		fclose(fp);
		return 0;
	}

	if(!(info_ptr = png_create_info_struct(png_ptr))) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(fp);
		return 0;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return 0;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, FILE_SIG_BYTES);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, 0);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &channel_bits, &color_type, &ilace_type, &compression, &filtering);
	*xsz = width;
	*ysz = height;
	if(!(pixels = malloc(width * height * sizeof *pixels))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return 0;
	}

	lineptr = (uint32_t**)png_get_rows(png_ptr, info_ptr);

	for(i=0; i<height; i++) {

		switch(color_type) {
		case PNG_COLOR_TYPE_RGB:
			{
				unsigned char *ptr = (unsigned char*)lineptr[i];
				for(j=0; j<width; j++) {

					uint32_t pixel;
					pixel = 0xff << 24 | ((uint32_t)*(ptr+2) << 16) | ((uint32_t)*(ptr+1) << 8) | (uint32_t)*ptr;
					ptr+=3;
					pixels[i * width + j] = pixel;
				}
			}
			break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
			memcpy(&pixels[i * width], lineptr[i], width * sizeof *pixels);
			break;

		default:
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			return 0;
		}

	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return pixels;
}
