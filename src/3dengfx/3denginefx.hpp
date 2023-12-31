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

#ifndef _3DENGINEFX_HPP_
#define _3DENGINEFX_HPP_

#include "opengl.h"
#include "3denginefx_types.hpp"
#include "textures.hpp"
#include "material.hpp"
#include "3dgeom.hpp"

extern Matrix4x4 world_matrix, view_matrix;

void CreateGraphicsContext(const GraphicsInitParameters &gip);
void DestroyGraphicsContext();
void SetDefaultStates();
const GraphicsInitParameters *GetGraphicsInitParameters();

void Clear(const Color &color);
void ClearZBuffer(scalar_t zval);
void ClearStencil(unsigned char sval);
void ClearZBufferStencil(scalar_t zval, unsigned char sval);

void Flip();

void LoadXFormMatrices();
void Draw(const VertexArray &varray);
void Draw(const VertexArray &varray, const IndexArray &iarray);

int GetTextureUnitCount();

////// render states //////
void SetPrimitiveType(PrimitiveType pt);
void SetBackfaceCulling(bool enable);
void SetFrontFace(FaceOrder order);
void SetAutoNormalize(bool enable);
//void SetBillboarding(bool enable);
void SetColorWrite(bool red, bool green, bool blue, bool alpha);
void SetWireframe(bool enable);

// blending states
void SetAlphaBlending(bool enable);
void SetBlendFunc(BlendingFactor src, BlendingFactor dest);

// zbuffer states
void SetZBuffering(bool enable);
void SetZWrite(bool enable);
void SetZFunc(CmpFunc func);

// set stencil buffer states
void SetStencilBuffering(bool enable);
void SetStencilPassOp(StencilOp sop);
void SetStencilFailOp(StencilOp sop);
void SetStencilPassZFailOp(StencilOp sop);
void SetStencilOp(StencilOp fail, StencilOp spass_zfail, StencilOp pass);
void SetStencilFunc(CmpFunc func);
void SetStencilReference(unsigned int ref);

// texture & material states
void SetTextureFiltering(int tex_unit, TextureFilteringType tex_filter);
void SetTextureAddressing(int tex_unit, TextureAddressing uaddr, TextureAddressing vaddr);
void SetTextureBorderColor(int tex_unit, const Color &color);
void SetTexture(int tex_unit, Texture *tex);
//void SetTextureFactor(dword factor);
void SetMipMapping(bool enable);
void SetMaterial(const Material &mat);

void SetRenderTarget(Texture *tex);

// multitexturing interface
void EnableTextureUnit(int tex_unit);
void DisableTextureUnit(int tex_unit);
void SetTextureUnitColor(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3 = TARG_NONE);
void SetTextureUnitAlpha(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3 = TARG_NONE);
void SetTextureCoordIndex(int tex_unit, int index);
void SetTextureConstant(int tex_unit, const Color &col);
//void SetTextureTransformState(int sttex_unitage, TexTransformState TexXForm);
//void SetTextureCoordGenerator(int stage, TexGen tgen);

// programmable interface
//TODO: implement the vertex/fragment program interface


// lighting states
void SetLighting(bool enable);
//void SetColorVertex(bool enable);
void SetAmbientLight(const Color &ambient_color);
void SetShadingMode(ShadeMode mode);
void SetSpecular(bool enable);

// Transformation Matrices
void SetMatrix(TransformType xform_type, const Matrix4x4 &mat, int num = 0);
Matrix4x4 GetMatrix(TransformType xform_type, int num = 0);
void SetViewport(unsigned int x, unsigned int y, unsigned int xsize, unsigned int ysize);

Matrix4x4 CreateProjectionMatrix(scalar_t vfov, scalar_t aspect, scalar_t near, scalar_t far);

// render target
//void ResetRenderTarget();
//void SetRenderTarget(RenderTarget &rtarg);
//void SetRenderTarget(Texture *rtarg, Texture *ztarg);
//RenderTarget GetRenderTarget() const;


////////////// function prototypes ///////////////
GraphicsInitParameters LoadGraphicsContextConfig(const char *fname);
void EngineLog(std::string log_data);
SysCaps GetSystemCapabilities();
const char *GetGLErrorString(GLenum error);
extern void (*LoadMatrixGL)(const Matrix4x4 &mat);

#endif	// _3DENGINEFX_HPP_
