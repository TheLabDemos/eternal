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

#include <iostream>
#include <list>
#include "opengl.h"
#include "SDL.h"
#include "3denginefx.hpp"
#include "3dgeom.hpp"
#include "except.hpp"
#include "logger.h"
#include "config_parser.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

void (*LoadMatrixGL)(const Matrix4x4 &mat);


#ifndef OPENGL_1_3
PFNGLLOADTRANSPOSEMATRIXFARBPROC glLoadTransposeMatrixf;
PFNGLACTIVETEXTUREARBPROC glActiveTexture;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;
#endif	// OPENGL_1_3

//#ifndef OPENGL_1_5
/* GL_ARB_vertex_array_object */
PFNGLBINDBUFFERARBPROC glBindBuffer;
PFNGLBUFFERDATAARBPROC glBufferData;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;
PFNGLISBUFFERARBPROC glIsBuffer;
PFNGLMAPBUFFERARBPROC glMapBuffer;
PFNGLUNMAPBUFFERARBPROC glUnmapBuffer;
PFNGLGENBUFFERSARBPROC glGenBuffers;
//#endif	/* OPENGL_1_5 */


static const char *gl_error_string[] = {
	"GL_INVALID_ENUM",		// 0x500
	"GL_INVALID_VALUE",		// 0x501
	"GL_INVALID_OPERATION",	// 0x502
	"GL_STACK_OVERFLOW",	// 0x503
	"GL_STACK_UNDERFLOW",	// 0x504
	"GL_OUT_OF_MEMORY",		// 0x505
	"GL_NO_ERROR",			// 0x0
	"[INVALID ERROR NUMBER]"
};

///////////////// local 3d engine state block ///////////////////
static GraphicsInitParameters gparams;
static SysCaps sys_caps;
Matrix4x4 world_matrix;
Matrix4x4 view_matrix;
static Matrix4x4 proj_matrix;
static Matrix4x4 tex_matrix[8];

static int coord_index[MAX_TEXTURES];

static PrimitiveType primitive_type;
static StencilOp stencil_fail, stencil_pass, stencil_pzfail;
static int stencil_ref;
static bool mipmapping = true;
static bool wire = false;

GraphicsInitParameters LoadGraphicsContextConfig(const char *fname) {
#ifdef _MSC_VER
	const char *__func__ = "LoadGraphicsContextConfig";
#endif	// _MSC_VER
	GraphicsInitParameters gip;	
	gip.x = 640;
	gip.y = 480;
	gip.bpp = 16;
	gip.depth_bits = 16;
	gip.stencil_bits = 8;
	gip.dont_care_flags = 0;
	
	if(LoadConfigFile(fname) == -1) {
		throw EngineException(__func__, "could not load config file");
	}
	
	char illegal_entry[100];
	sprintf(illegal_entry, "error parsing config file %s", fname);
	
	const ConfigOption *cfgopt;
	while((cfgopt = GetNextOption())) {
		
		if(!strcmp(cfgopt->option, "fullscreen")) {
			if(!strcmp(cfgopt->str_value, "true")) {
				gip.fullscreen = true;
			} else if(!strcmp(cfgopt->str_value, "false")) {
				gip.fullscreen = false;
			} else {
				throw EngineException(__func__, illegal_entry);
			}
		} else if(!strcmp(cfgopt->option, "resolution")) {
			if(!isdigit(cfgopt->str_value[0])) {
				throw EngineException(__func__, illegal_entry);
			}
			gip.x = atoi(cfgopt->str_value);
			
			char *ptr = cfgopt->str_value;
			while(*ptr && *ptr != 'x') *ptr++;
			if(!*ptr || !*(ptr+1) || !isdigit(*(ptr+1))) {
				throw EngineException(__func__, illegal_entry);
			}
			
			gip.y = atoi(ptr + 1);
		} else if(!strcmp(cfgopt->option, "bpp")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.bpp = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.bpp = 32;
				gip.dont_care_flags |= DONT_CARE_BPP;
			} else {
				throw EngineException(__func__, illegal_entry);
			}			
		} else if(!strcmp(cfgopt->option, "zbuffer")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.depth_bits = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.depth_bits = 32;
				gip.dont_care_flags |= DONT_CARE_DEPTH;
			} else {
				throw EngineException(__func__, illegal_entry);
			}
		} else if(!strcmp(cfgopt->option, "stencil")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.stencil_bits = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.stencil_bits = 8;
				gip.dont_care_flags |= DONT_CARE_STENCIL;
			} else {
				throw EngineException(__func__, illegal_entry);
			}
		}
	}
	
	DestroyConfigParser();
	
	return gip;		
}

/* ---- EngineLog(string) ----
 * handles uniform logging for 3D engine messages
 */
void EngineLog(std::string log_data) {
	std::string log_str = std::string("[3DENGFX] ") + log_data;
	Log("3dengfx.log", log_str.c_str());
}


/* ---- GetSystemCapabilities() ----
 * Retrieves information on the graphics subsystem capabilities
 * and returns a SysCaps structure describing them
 */
SysCaps GetSystemCapabilities() {
	static bool first_call = true;
	
	if(!first_call) {
		return sys_caps;
	}
	first_call = false;
	
	// get extensions & vendor strings
	char *ext_str = strdup((const char*)glGetString(GL_EXTENSIONS));
	char *cptr = ext_str;
	
	while(*cptr) {
		if(*cptr == ' ') *cptr = '\n';
		cptr++;
	}
	Log("gl_ext.log", "Supported extensions:\n-------------\n");
	Log("gl_ext.log", ext_str);
	Log("gl_ext.log", "\n");
	
	char *vendor_str = strdup((const char*)glGetString(GL_VENDOR));
	char *renderer_str = strdup((const char*)glGetString(GL_RENDERER));
	char *version_str = strdup((const char*)glGetString(GL_VERSION));
	
	EngineLog("Rendering System Information:\n");
	EngineLog("  Vendor: " + std::string(vendor_str) + "\n");
	EngineLog("Renderer: " + std::string(renderer_str) + "\n");
	EngineLog(" Version: " + std::string(version_str) + "\n");
	EngineLog("(note: the list of extensions is logged seperately at \"gl_ext.log\")\n");	

	free(vendor_str);
	free(renderer_str);
	free(version_str);
	
	// fill the SysCaps structure
	SysCaps sys_caps;
	sys_caps.load_transpose = (bool)strstr(ext_str, "GL_ARB_transpose_matrix");
	sys_caps.gen_mipmaps = (bool)strstr(ext_str, "GL_SGIS_generate_mipmap");
	sys_caps.tex_combine_ops = (bool)strstr(ext_str, "GL_ARB_texture_env_combine");
	sys_caps.bump_dot3 = (bool)strstr(ext_str, "GL_ARB_texture_env_dot3");
	sys_caps.bump_env = (bool)strstr(ext_str, "GL_ATI_envmap_bumpmap");
	sys_caps.vertex_buffers = (bool)strstr(ext_str, "GL_ARB_vertex_buffer_object");
	sys_caps.depth_texture = (bool)strstr(ext_str, "GL_ARB_depth_texture");
	sys_caps.shadow_mapping = (bool)strstr(ext_str, "GL_ARB_shadow");
	sys_caps.vertex_program = (bool)strstr(ext_str, "GL_ARB_vertex_program");
	sys_caps.pixel_program = (bool)strstr(ext_str, "GL_ARB_fragment_program");
	sys_caps.glslang = (bool)strstr(ext_str, "GL_ARB_shading_language_100");
	sys_caps.point_sprites = (bool)strstr(ext_str, "GL_ARB_point_sprites");
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &sys_caps.max_texture_units);
	
	// also log these things
	EngineLog("-------------------\n");
	EngineLog("System Capabilities\n");
	EngineLog("-------------------\n");
	EngineLog("Load transposed matrices: " + string(sys_caps.load_transpose ? "yes\n" : "no\n"));
	EngineLog("Auto-generate mipmaps (SGIS): " + string(sys_caps.gen_mipmaps ? "yes\n" : "no\n"));
	EngineLog("Custom texture combination operations: " + string(sys_caps.tex_combine_ops ? "yes\n" : "no\n"));
	EngineLog("Diffuse bump mapping (dot3): " + string(sys_caps.bump_dot3 ? "yes\n" : "no\n"));
	EngineLog("Specular bump mapping (env-bump): " + string(sys_caps.bump_env ? "yes\n" : "no\n"));
	EngineLog("Video memory vertex/index buffers: " + string(sys_caps.vertex_buffers ? "yes\n" : "no\n"));
	EngineLog("Depth texture: " + string(sys_caps.depth_texture ? "yes\n" : "no\n"));
	EngineLog("Shadow mapping: " + string(sys_caps.shadow_mapping ? "yes\n" : "no\n"));
	EngineLog("Programmable vertex processing: " + string(sys_caps.vertex_program ? "yes\n" : "no\n"));
	EngineLog("Programmable pixel processing: " + string(sys_caps.pixel_program ? "yes\n" : "no\n"));
	EngineLog("OpenGL 2.0 shading language: " + string(sys_caps.glslang ? "yes\n" : "no\n"));
	EngineLog("Point sprites: " + string(sys_caps.point_sprites ? "yes\n" : "no\n"));
	char tex_units_str[10];
	sprintf(tex_units_str, "%d\n", sys_caps.max_texture_units);
	EngineLog("Texture units: " + string(tex_units_str));
	
	return sys_caps;
}

const char *GetGLErrorString(GLenum error) {
	if(!error) return gl_error_string[0x506];
	if(error < 0x500 || error > 0x505) error = 0x507;
	return gl_error_string[error - 0x500];
}

/* LoadMatrix_TransposeARB() & LoadMatrix_TransposeManual()
 * --------------------------------------------------------
 * two functions to handle the transformation matrix loading
 * to OpenGL by either transposing the Matrix4x4 data or using
 * the transposed-loading extension (use through function pointer
 * LoadMatrixGL which is set during initialization to the correct one)
 */
void LoadMatrix_TransposeARB(const Matrix4x4 &mat) {
	glLoadTransposeMatrixf(mat.OpenGLMatrix());
}

void LoadMatrix_TransposeManual(const Matrix4x4 &mat) {
	glLoadMatrixf(mat.Transposed().OpenGLMatrix());
}


//////////////// 3D Engine Initialization ////////////////

/* ---- CreateGraphicsContext() ----
 * initializes the graphics subsystem according to the init parameters
 */
void CreateGraphicsContext(const GraphicsInitParameters &gip) {
	
	gparams = gip;

	remove("3dengfx.log");
	remove("gl_ext.log");	
	EngineLog("Initializing SDL\n");
	
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) == -1) {
		throw EngineException("GraphicsContext::GraphicsContext()", "Could not initialize SDL library.");
	}

	if(gparams.fullscreen) SDL_ShowCursor(0);
	
	if(!gparams.fullscreen) {
		const SDL_VideoInfo *vid_inf = SDL_GetVideoInfo();
		gparams.bpp = vid_inf->vfmt->BitsPerPixel;
	}
	
	char video_mode[128];
	sprintf(video_mode, "%dx%dx%d, d:%d s:%d %s", gparams.x, gparams.y, gparams.bpp, gparams.depth_bits, gparams.stencil_bits, gparams.fullscreen ? "fullscreen" : "windowed");
	EngineLog("Trying to set Video Mode " + std::string(video_mode) + "\n");
	
	int rbits, gbits, bbits;
	switch(gparams.bpp) {
	case 32:
		rbits = gbits = bbits = 8;
		break;
		
	case 16:
		rbits = bbits = 6;
		gbits = 5;
		break;
		
	default:
		char bpp_str[32];
		sprintf(bpp_str, "%d", gparams.bpp);
		throw EngineException("GraphicsContext::GraphicsContext()", "Tried to set unsupported pixel format: " + std::string(bpp_str) + " bpp");
	}
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rbits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gbits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bbits);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gparams.depth_bits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, gparams.stencil_bits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	unsigned long flags = SDL_OPENGL;
	if(gparams.fullscreen) flags |= SDL_FULLSCREEN;
	if(!SDL_SetVideoMode(gparams.x, gparams.y, gparams.bpp, flags)) {
		if(gparams.depth_bits == 32) gparams.depth_bits = 24;
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gparams.depth_bits);
		
		if(!SDL_SetVideoMode(gparams.x, gparams.y, gparams.bpp, flags)) {
			throw EngineException("GraphicsContext::GraphicsContext()", "Could not set requested video mode");
		}
	}
	
	// now check the actual video mode we got
	int arbits, agbits, abbits, azbits, astencilbits;
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &arbits);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &agbits);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &abbits);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &azbits);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &astencilbits);

	char bppstr[32];
	sprintf(bppstr, "%d (%d%d%d)", arbits + agbits + abbits, arbits, agbits, abbits);
	char zstr[10];
	sprintf(zstr, "%d", azbits);
	char stencilstr[10];
	sprintf(stencilstr, "%d", astencilbits);
	
	EngineLog("Initialized Video Mode:\n");
	EngineLog("    bpp: " + std::string(bppstr) + "\n");
	EngineLog("zbuffer: " + std::string(zstr) + "\n");
	EngineLog("stencil: " + std::string(stencilstr) + "\n");

	/* if the dont_care_flags does not contain DONT_CARE_BPP and our color bits
	 * does not match, we should throw the exception, however we test against
	 * the difference allowing a +/-1 difference in order to allow for 16bpp
	 * formats of either 565 or 555 and consider them equal.
	 */
	if(!(gparams.dont_care_flags & DONT_CARE_BPP) && abs(arbits - rbits) > 1 && abs(agbits - gbits) > 1 && abs(abbits - bbits) > 1) {
		throw EngineException("GraphicsContext::GraphicsContext()", "Could not set requested exact bpp mode");
	}
	
	// now if we don't have DONT_CARE_DEPTH in the dont_care_flags check for 
	// exact depth buffer format, however consider 24 and 32 bit the same
	if(!(gparams.dont_care_flags & DONT_CARE_DEPTH) && azbits != gparams.depth_bits) {
		if(!(gparams.depth_bits == 32 && azbits == 24 || gparams.depth_bits == 24 && azbits == 32)) {
			throw EngineException("GraphicsContext::GraphicsContext()", "Could not set requested exact zbuffer depth");
		}
	}
	
	// if we don't have DONT_CARE_STENCIL make sure we have the stencil format
	// that was asked.
	if(!(gparams.dont_care_flags & DONT_CARE_STENCIL) && astencilbits != gparams.stencil_bits) {
		throw EngineException("GraphicsContext::GraphicsContext()", "Could not set exact stencil format");
	}
	
	sys_caps = GetSystemCapabilities();
	if(sys_caps.max_texture_units < 2) {
		throw EngineException("GraphicsContext::GraphicsContext()", "Your system does not meet the minimum requirements (at least 2 texture units)");
	}

#ifndef OPENGL_1_3	
	if(sys_caps.load_transpose) {
		glLoadTransposeMatrixf = (PFNGLLOADTRANSPOSEMATRIXFARBPROC)SDL_GL_GetProcAddress("glLoadTransposeMatrixfARB");
		LoadMatrixGL = LoadMatrix_TransposeARB;
	} else {
		LoadMatrixGL = LoadMatrix_TransposeManual;
	}

	glActiveTexture = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glClientActiveTextureARB");
	if(!glActiveTexture || !glClientActiveTexture) std::cerr << "los poulos\n";
#else
	LoadMatrixGL = LoadMatrix_TransposeARB;
#endif	// OPENGL_1_3

//#ifndef OPENGL_1_5
	if(sys_caps.vertex_buffers) {
		glBindBuffer = (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
		glBufferData = (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
		glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");
		glIsBuffer = (PFNGLISBUFFERARBPROC)SDL_GL_GetProcAddress("glIsBufferARB");
		glMapBuffer = (PFNGLMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glMapBufferARB");
		glUnmapBuffer = (PFNGLUNMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glUnmapBufferARB");
		glGenBuffers = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
	}
//#endif	// OPENGL_1_5
	
	SetDefaultStates();	
}

void DestroyGraphicsContext() {
	if(gparams.fullscreen) SDL_ShowCursor(1);
	SDL_Quit();
}

void SetDefaultStates() {
	SetPrimitiveType(TRIANGLE_LIST);
	SetFrontFace(ORDER_CW);
	SetBackfaceCulling(true);
	SetZBuffering(true);
	SetLighting(true);
	SetAutoNormalize(false);
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	
	SetMatrix(XFORM_WORLD, Matrix4x4());
	SetMatrix(XFORM_VIEW, Matrix4x4());
	SetMatrix(XFORM_PROJECTION, CreateProjectionMatrix(quarter_pi, 1.333333f, 1.0f, 1000.0f));
	
	memset(coord_index, 0, MAX_TEXTURES * sizeof(int));
}

const GraphicsInitParameters *GetGraphicsInitParameters() {
	return &gparams;
}

void Clear(const Color &color) {
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void ClearZBuffer(scalar_t zval) {
	glClearDepth(zval);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void ClearStencil(unsigned char sval) {
	glClearStencil(sval);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void ClearZBufferStencil(scalar_t zval, unsigned char sval) {
	glClearDepth(zval);
	glClearStencil(sval);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Flip() {
	SDL_GL_SwapBuffers();
}

void LoadXFormMatrices() {
	glMatrixMode(GL_PROJECTION);
	LoadMatrixGL(proj_matrix);
	
	Matrix4x4 modelview = view_matrix * world_matrix;
	glMatrixMode(GL_MODELVIEW);
	LoadMatrixGL(modelview);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void Draw(const VertexArray &varray) {
	LoadXFormMatrices();

	bool use_vbo = !varray.GetDynamic() && sys_caps.vertex_buffers;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	if(use_vbo) {
		Vertex v;
		glBindBuffer(GL_ARRAY_BUFFER_ARB, varray.GetBufferObject());
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)((char*)&v.pos - (char*)&v));
		glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)((char*)&v.normal - (char*)&v));
		glColorPointer(4, GL_FLOAT, sizeof(Vertex), (void*)((char*)&v.color - (char*)&v));

		for(int i=0; i<MAX_TEXTURES; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)((char*)&v.tex[coord_index[i]] - (char*)&v));
		}
	} else {
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &varray.GetData()->pos);
		glNormalPointer(GL_FLOAT, sizeof(Vertex), &varray.GetData()->normal);
		glColorPointer(4, GL_FLOAT, sizeof(Vertex), &varray.GetData()->color);

		for(int i=0; i<MAX_TEXTURES; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &varray.GetData()->tex[coord_index[i]]);
		}
	}
	
	glDrawArrays(primitive_type, 0, varray.GetCount());
	
	if(use_vbo) glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	for(int i=0; i<MAX_TEXTURES; i++) {
		glClientActiveTexture(GL_TEXTURE0 + i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void Draw(const VertexArray &varray, const IndexArray &iarray) {
	static int dbg;
	dbg++;
	LoadXFormMatrices();

	bool use_vbo = !varray.GetDynamic() && sys_caps.vertex_buffers;
	bool use_ibo = false;//!iarray.GetDynamic() && sys_caps.vertex_buffers;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	if(use_vbo) {
		Vertex v;
		glBindBuffer(GL_ARRAY_BUFFER_ARB, varray.GetBufferObject());
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)((char*)&v.pos - (char*)&v));
		glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)((char*)&v.normal - (char*)&v));
		glColorPointer(4, GL_FLOAT, sizeof(Vertex), (void*)((char*)&v.color - (char*)&v));

		for(int i=0; i<MAX_TEXTURES; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)((char*)&v.tex[coord_index[i]] - (char*)&v));
		}
	} else {
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &varray.GetData()->pos);
		glNormalPointer(GL_FLOAT, sizeof(Vertex), &varray.GetData()->normal);
		glColorPointer(4, GL_FLOAT, sizeof(Vertex), &varray.GetData()->color);

		for(int i=0; i<MAX_TEXTURES; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &varray.GetData()->tex[coord_index[i]]);
		}
	}

	if(use_ibo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, iarray.GetBufferObject());
		glDrawElements(primitive_type, iarray.GetCount(), GL_UNSIGNED_SHORT, 0);
	} else {
		glDrawElements(primitive_type, iarray.GetCount(), GL_UNSIGNED_SHORT, iarray.GetData());
	}
	
	if(use_ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	if(use_vbo) glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	for(int i=0; i<MAX_TEXTURES; i++) {
		glClientActiveTexture(GL_TEXTURE0 + i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

int GetTextureUnitCount() {
	return sys_caps.max_texture_units;
}

//////////////////// render states /////////////////////

void SetPrimitiveType(PrimitiveType pt) {
	primitive_type = pt;
}

void SetBackfaceCulling(bool enable) {
	if(enable) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
}

void SetFrontFace(FaceOrder order) {
	glFrontFace(order);
}

void SetAutoNormalize(bool enable) {
	if(enable) {
		glEnable(GL_NORMALIZE);
	} else {
		glDisable(GL_NORMALIZE);
	}
}

void SetColorWrite(bool red, bool green, bool blue, bool alpha) {
	glColorMask(red, green, blue, alpha);
}

void SetWireframe(bool enable) {
	SetPrimitiveType(enable ? LINE_LIST : TRIANGLE_LIST);
}
	

///////////////// blending states ///////////////

void SetAlphaBlending(bool enable) {
	if(enable) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
}

void SetBlendFunc(BlendingFactor src, BlendingFactor dest) {
	glBlendFunc(src, dest);
}

///////////////// zbuffer states ////////////////

void SetZBuffering(bool enable) {
	if(enable) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
}

void SetZWrite(bool enable) {
	glDepthMask(enable);
}

void SetZFunc(CmpFunc func) {
	glDepthFunc(func);
}

/////////////// stencil states //////////////////
void SetStencilBuffering(bool enable) {
	if(enable) {
		glEnable(GL_STENCIL_TEST);
	} else {
		glDisable(GL_STENCIL_TEST);
	}
}

void SetStencilPassOp(StencilOp sop) {
	stencil_pass = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void SetStencilFailOp(StencilOp sop) {
	stencil_fail = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void SetStencilPassZFailOp(StencilOp sop) {
	stencil_pzfail = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void SetStencilOp(StencilOp fail, StencilOp spass_zfail, StencilOp pass) {
	stencil_fail = fail;
	stencil_pzfail = spass_zfail;
	stencil_pass = pass;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void SetStencilFunc(CmpFunc func) {
	glStencilFunc(func, stencil_ref, 0xffffffff);
}

void SetStencilReference(unsigned int ref) {
	stencil_ref = ref;
}

///////////// texture & material states //////////////
void SetTextureFiltering(int tex_unit, TextureFilteringType tex_filter) {
	
	int min_filter;
	
	switch(tex_filter) {
	case POINT_SAMPLING:
		min_filter = mipmapping ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
		
	case BILINEAR_FILTERING:
		min_filter = mipmapping ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
		
	case TRILINEAR_FILTERING:
	default:
		min_filter = mipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
}

void SetTextureAddressing(int tex_unit, TextureAddressing uaddr, TextureAddressing vaddr) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uaddr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vaddr);
}

void SetTextureBorderColor(int tex_unit, const Color &color) {
	float col[] = {color.r, color.g, color.b, color.a};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, col);
}

void SetTexture(int tex_unit, Texture *tex) {
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(GL_TEXTURE_2D, tex->tex_id);
}

void SetMipMapping(bool enable) {
	mipmapping = true;
}

void SetMaterial(const Material &mat) {
	mat.SetGLMaterial();
}

void SetRenderTarget(Texture *tex) {
	static Texture *prev;
	
	if(tex == prev) return;

	if(prev) {
		SetTexture(0, prev);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, prev->width, prev->height);
		glGetError();	// swallow
	}
	
	if(!tex) {
		SetViewport(0, 0, gparams.x, gparams.y);
	} else {
		SetViewport(0, 0, tex->width, tex->height);
	}

	prev = tex;
}		

// multitexturing interface

void EnableTextureUnit(int tex_unit) {
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glEnable(GL_TEXTURE_2D);
}

void DisableTextureUnit(int tex_unit) {
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glDisable(GL_TEXTURE_2D);
}

void SetTextureUnitColor(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3) {
	
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, op);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, arg1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, arg2);
	if(arg3 != TARG_NONE) {
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, arg3);
	}
}

void SetTextureUnitAlpha(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3) {
	
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, op);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, arg1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, arg2);
	if(arg3 != TARG_NONE) {
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, arg3);
	}
}

void SetTextureCoordIndex(int tex_unit, int index) {
	coord_index[tex_unit] = index;
}

void SetTextureConstant(int tex_unit, const Color &col) {
	float color[] = {col.r, col.g, col.b, col.a};
	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
}

//void SetTextureTransformState(int sttex_unitage, TexTransformState TexXForm);
//void SetTextureCoordGenerator(int stage, TexGen tgen);


// lighting states
void SetLighting(bool enable) {
	if(enable) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}
}

void SetAmbientLight(const Color &ambient_color) {
	float col[] = {ambient_color.r, ambient_color.g, ambient_color.b, ambient_color.a};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, col);
}

void SetShadingMode(ShadeMode mode) {
	glShadeModel(mode);
}

// transformation matrices
void SetMatrix(TransformType xform_type, const Matrix4x4 &mat, int num) {
	switch(xform_type) {
	case XFORM_WORLD:
		world_matrix = mat;
		break;
		
	case XFORM_VIEW:
		view_matrix = mat;
		break;
		
	case XFORM_PROJECTION:
		proj_matrix = mat;
		break;
		
	case XFORM_TEXTURE:
		tex_matrix[num] = mat;
		break;
	}
}

Matrix4x4 GetMatrix(TransformType xform_type, int num) {
	switch(xform_type) {
	case XFORM_WORLD:
		return world_matrix;
		
	case XFORM_VIEW:
		return view_matrix;
		
	case XFORM_TEXTURE:
		return tex_matrix[num];
		
	case XFORM_PROJECTION:
	default:
		return proj_matrix;
	}
}

void SetViewport(unsigned int x, unsigned int y, unsigned int xsize, unsigned int ysize) {
	glViewport(x, y, xsize, ysize);
}

Matrix4x4 CreateProjectionMatrix(scalar_t vfov, scalar_t aspect, scalar_t near_clip, scalar_t far_clip) {
	
	scalar_t hfov = vfov * aspect;
	scalar_t w = 1.0f / (scalar_t)tan(hfov * 0.5f);
	scalar_t h = 1.0f / (scalar_t)tan(vfov * 0.5f);
	scalar_t q = far_clip / (far_clip - near_clip);
	
	Matrix4x4 mat;
	//mat.SetScaling(Vector4(w, h, q, 0));
	mat[0][0] = w;
	mat[1][1] = h;
	mat[2][2] = q;
	mat[3][2] = 1.0f;
	mat[2][3] = -q * near_clip;
	
	return mat;
}
