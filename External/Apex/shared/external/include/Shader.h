/*
 * Copyright 2009-2011 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */
#ifndef SHADER_H
#define SHADER_H

#include "PsShare.h"
#include "PxVec3.h"
#include "PxMat34Legacy.h"
#include <map>

#if defined(PX_WINDOWS)
#include <GL/glew.h>

enum Shader_Mode
{
	SM_GpuSkin = (1 << 0),
	SM_Wireframe = (1 << 1),
	SM_NoLighting = (1 << 2),
	SM_Wind = (1 << 3),
	SM_Texture = (1 << 4),
	SM_Geometry = (1 << 5),
	SM_VertexColor = (1 << 6),

	NUM_SHADER_BITS = 7,
	NUM_SHADER_MODES = (1 << NUM_SHADER_BITS),
};

class Shader
{
protected:
	GLuint mShaderProgram[NUM_SHADER_MODES];
	GLuint mVertexShader[NUM_SHADER_MODES];
	GLuint mGeometryShader[NUM_SHADER_MODES];
	GLuint mFragmentShader[NUM_SHADER_MODES];
	GLuint mGlShaderAttributes[NUM_SHADER_MODES];

	enum glShaderAttribute
	{
		gl_VERTEX	= (1 << 0),
		gl_NORMAL	= (1 << 1),
		gl_COLOR	= (1 << 2),
		gl_TEXTURE	= (1 << 3),
	};

	physx::PxU32 mCurrentMode;
	static Shader* mActiveShader;

	const char* mVertexShaderFile;
	const char* mGeometryShaderFile;
	const char* mFragmentShaderFile;

	struct UniformVariable
	{
		GLint size;
		GLenum type;
		GLint index;
	};

	struct AttributeVariable
	{
		GLint size;
		GLenum type;
		GLint index;
	};

	typedef std::map<std::string, UniformVariable> tUniforms;
	tUniforms mUniforms[NUM_SHADER_MODES];
	typedef std::map<std::string, AttributeVariable> tAttributes;
	tAttributes mAttributes[NUM_SHADER_MODES];

	char* mVertexShaderSource;
	int   mVertexShaderSourceLength;
	char* mGeometryShaderSource;
	int   mGeometryShaderSourceLength;
	GLenum mGeometryShaderInputType;
	GLenum mGeometryShaderOutputType;
	int mGeometryShaderNumVerticesOutput;
	
	char* mFragmentShaderSource;
	int   mFragmentShaderSourceLength;

	bool compileShaderMode(physx::PxU32 shaderMode);

	void printCompileError(GLuint shader, bool ok, const char* shaderFile);
	void printLinkError(GLuint shader, bool ok);

	static physx::PxU32 mMaxBones;

	bool mErrorMessageGui;
public:
	Shader(const char* vertexShader, const char* fragmentShader, bool isResource, bool errorMessageGui);
	Shader(const char* vertexShader, const char* geometryShader, GLenum geomInput, GLenum geomOutput, int geomNumVerticesOutput,
		const char* fragmentShader, bool isResource, bool errorMessageGui);
	~Shader();

	static physx::PxU32 shaderSwitch;

	bool compileShaderModes(physx::PxU32* shaderModes, physx::PxU32 numShaderModes);

	void activate(physx::PxU32 shaderMode);
	physx::PxU32 deactivate();

	bool addMode(Shader_Mode mode);
	void removeMode(Shader_Mode mode);

	bool isModeActive(Shader_Mode mode) { return (mCurrentMode & mode) != 0; }

	static Shader* peek() { return mActiveShader; }
	static physx::PxU32 getMaxBones();
	bool isValid(physx::PxU32 shaderMode);

	bool setUniform(const char* name, const physx::PxMat33Legacy& value);
	bool setUniform(const char* name, const physx::PxMat34Legacy& value);
	bool setUniform(const char *name, physx::PxU32 size, const physx::PxVec3* value);
	bool setUniform1(const char* name, physx::PxU32 size, const float* value);
	bool setUniform3x4(const char *name, physx::PxU32 size, const physx::PxMat34Legacy* value);
	bool setUniform4x4(const char *name, physx::PxU32 size, const float* value);

	bool setAttribute(const char* name, physx::PxU32 size, physx::PxU32 stride, GLenum type, void* data);
private:
	void init(const char* vertexShader, const char* geometryShader, GLenum geomInput, GLenum geomOutput, int geomNumVerticesOutput,
		      const char* fragmentShader, bool isResource);
	void loadFile(const char* filename, bool isResource, char*& destination, int& destinationLength);

	GLint getUniformCommon(const char* name);
};

#endif // PX_WINDOWS

#endif