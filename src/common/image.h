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
#ifndef IMAGE_H_
#define IMAGE_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void *LoadImage(const char *fname, unsigned long *xsz, unsigned long *ysz);

#ifdef __cplusplus
}
#endif	/* __cplusplus */
	
#endif	/* IMAGE_H_ */
