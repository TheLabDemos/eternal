/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the eternal demo.

The eternal library is free software; you can redistribute it and/or modify
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
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif	/* _cplusplus */

void SetLogFile(const char *fname);

/*
** void Log(const char *fname, const char *log_data);
** void Log(const char *log_data);
*/
void Log(const char *str1, ...);

#ifdef __cplusplus
}
#endif	/* _cplusplus */

#endif	/* _LOGGER_H_ */
