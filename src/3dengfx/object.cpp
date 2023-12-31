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

#include "opengl.h"
#include "object.hpp"
#include "3denginefx.hpp"


Object::Object() {
	render_params.shading = SHADING_GOURAUD;
	render_params.billboarded = false;
	render_params.zwrite = true;
	render_params.blending = false;
	render_params.src_blend = BLEND_SRC_ALPHA;
	render_params.dest_blend = BLEND_ONE_MINUS_SRC_ALPHA;
}

Object::Object(const TriMesh &mesh) {
	render_params.shading = SHADING_GOURAUD;
	render_params.billboarded = false;
	render_params.zwrite = true;
	render_params.blending = false;
	render_params.src_blend = BLEND_SRC_ALPHA;
	render_params.dest_blend = BLEND_ONE_MINUS_SRC_ALPHA;
	
	this->mesh = mesh;
}

void Object::SetTriMesh(const TriMesh &mesh) {
	this->mesh = mesh;
}

TriMesh *Object::GetTriMeshPtr() {
	return &mesh;
}

TriMesh Object::GetTriMesh() const {
	return mesh;
}

void Object::SetDynamic(bool enable) {
	const_cast<VertexArray*>(mesh.GetVertexArray())->SetDynamic(enable);
	const_cast<TriangleArray*>(mesh.GetTriangleArray())->SetDynamic(enable);
	//const_cast<IndexArray*>(mesh.GetIndexArray())->SetDynamic(enable);
}

bool Object::GetDynamic() const {
	return mesh.GetVertexArray()->GetDynamic();
}

void Object::SetMaterial(const Material &mat) {
	this->mat = mat;
}

Material *Object::GetMaterialPtr() {
	return &mat;
}

Material Object::GetMaterial() const {
	return mat;
}

void Object::SetRenderParams(const RenderParams &rp) {
	render_params = rp;
}

RenderParams Object::GetRenderParams() const {
	return render_params;
}

void Object::SetShading(ShadeMode shading_mode) {
	render_params.shading = shading_mode;
}

void Object::SetBillboarding(bool enable) {
	render_params.billboarded = enable;
}

void Object::SetZWrite(bool enable) {
	render_params.zwrite = enable;
}

void Object::SetBlending(bool enable) {
	render_params.blending = enable;
}

void Object::SetBlendingMode(BlendingFactor sblend, BlendingFactor dblend) {
	render_params.src_blend = sblend;
	render_params.dest_blend = dblend;
}

void Object::SetWireframe(bool enable) {
	render_params.wire = enable;
}

void Object::Render8TexUnits() {
	
	int tex_unit = 0;
	unsigned int tex_id = 0;

	::SetMaterial(mat);
	
	if(mat.tex[TEXTYPE_ENVMAP]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		//SetTextureConstant(tex_unit, Color(mat.env_intensity));
		//SetTextureUnitColor(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_CONSTANT);
		SetTextureUnitColor(tex_unit, TOP_REPLACE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_TEXTURE);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		SetTexture(tex_unit, mat.tex[TEXTYPE_ENVMAP]);
		tex_id = mat.tex[TEXTYPE_ENVMAP]->tex_id;
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_DIFFUSE]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		SetTextureUnitColor(tex_unit, tex_unit ? TOP_ADD : TOP_REPLACE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_REPLACE, TARG_TEXTURE, TARG_PREV);
		SetTexture(tex_unit, mat.tex[TEXTYPE_DIFFUSE]);
		tex_id = mat.tex[TEXTYPE_DIFFUSE]->tex_id;
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_DETAIL]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 1);
		SetTextureUnitColor(tex_unit, tex_unit ? TOP_ADD : TOP_REPLACE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_ADD, TARG_TEXTURE, TARG_PREV);
		SetTexture(tex_unit, mat.tex[TEXTYPE_DETAIL]);
		tex_id = mat.tex[TEXTYPE_DETAIL]->tex_id;
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_LIGHTMAP]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		SetTextureUnitColor(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_TEXTURE);
		SetTexture(tex_unit, mat.tex[TEXTYPE_LIGHTMAP]);
		tex_id = mat.tex[TEXTYPE_LIGHTMAP]->tex_id;
		tex_unit++;
	}
	
	if(tex_unit) {
		EnableTextureUnit(tex_unit);
		SetTextureUnitColor(tex_unit, TOP_MODULATE, TARG_COLOR, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_MODULATE, TARG_COLOR, TARG_PREV);
		glBindTexture(GL_TEXTURE_2D, tex_id);
		tex_unit++;
	}

	::SetZWrite(render_params.zwrite);
	SetShadingMode(render_params.shading);
	SetAlphaBlending(render_params.blending);
	SetBlendFunc(render_params.src_blend, render_params.dest_blend);
	::SetWireframe(render_params.wire);
	
	Draw(*mesh.GetVertexArray(), *mesh.GetIndexArray());

	if(render_params.wire) ::SetWireframe(false);
	if(render_params.blending) SetAlphaBlending(false);
	if(render_params.zwrite) ::SetZWrite(true);
	if(render_params.shading == SHADING_FLAT) SetShadingMode(SHADING_GOURAUD);
	
	for(int i=0; i<tex_unit; i++) {
		DisableTextureUnit(i);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
}
	

void Object::Render(unsigned long time) {
	world_mat = GetPRS(time).GetXFormMatrix();
	
	SetMatrix(XFORM_WORLD, world_mat);
	mat.SetGLMaterial();
	
	//Render8TexUnits();
	RenderHack();
}

void Object::RenderHack() {
	::SetMaterial(mat);
	int tex_unit = 0;

	if(mat.tex[TEXTYPE_DIFFUSE]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		SetTextureUnitColor(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		SetTexture(tex_unit, mat.tex[TEXTYPE_DIFFUSE]);
		//tex_id = mat.tex[TEXTYPE_DIFFUSE]->tex_id;
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_ENVMAP]) {
		EnableTextureUnit(tex_unit);
		SetTextureUnitColor(tex_unit, TOP_ADD, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_MODULATE, TARG_PREV, TARG_TEXTURE);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		SetTexture(tex_unit, mat.tex[TEXTYPE_ENVMAP]);
		//tex_id = mat.tex[TEXTYPE_ENVMAP]->tex_id;
		tex_unit++;
	}
	
	::SetZWrite(render_params.zwrite);
	SetShadingMode(render_params.shading);
	SetAlphaBlending(render_params.blending);
	//SetAlphaBlending(true);
	SetBlendFunc(render_params.src_blend, render_params.dest_blend);
	
	Draw(*mesh.GetVertexArray(), *mesh.GetIndexArray());

	//SetAlphaBlending(false);
	if(render_params.blending) SetAlphaBlending(false);
	if(render_params.zwrite) ::SetZWrite(true);
	if(render_params.shading == SHADING_FLAT) SetShadingMode(SHADING_GOURAUD);
	
	for(int i=0; i<tex_unit; i++) {
		DisableTextureUnit(i);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
}
