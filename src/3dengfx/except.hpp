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

#ifndef _EXCEPT_HPP_
#define _EXCEPT_HPP_

#include <string>

#ifdef GetMessage
#undef GetMessage
#endif	// GetMessage

class EngineException {
protected:
	std::string reason, source;

public:
	EngineException(std::string src, std::string reason = "undefined");
	
	std::string GetMessage() const;
};

#endif	// _EXCEPT_HPP_
