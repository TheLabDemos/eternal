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
#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_


#ifdef __cplusplus
extern "C" {
#endif	/* _cplusplus */


enum ParserState {
	PS_AssignmentSymbol,
	PS_CommentSymbol,
	PS_Seperators,
	PS_MaxLineLen
};

#define CFGOPT_INT	1
#define CFGOPT_FLT	2
	
struct ConfigOption {
	char *option, *str_value;
	int int_value;
	float flt_value;
	unsigned short flags;
};

void SetParserState(enum ParserState state, int value);
int LoadConfigFile(const char *fname);
const struct ConfigOption *GetNextOption();
void DestroyConfigParser();


#ifdef __cplusplus
}
#endif	/* _cplusplus */

#endif	/* _CONFIG_PARSER_H_ */
