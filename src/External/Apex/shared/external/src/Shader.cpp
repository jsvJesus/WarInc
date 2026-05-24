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
#include "PsShare.h"
#include "PxPreprocessor.h"
#include "Shader.h"

#if defined(PX_PS3)
#warning "Shader.cpp not implemented on PS3"
#elif defined(PX_X360)
#pragma message("Shader.cpp not implemented on Xbox360")
#else

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#include <cstdio>
#include <string>

#define NOMINMAX
#include <windows.h>

#define DEBUG_SHADERS 0


Shader* Shader::mActiveShader = NULL;
physx::PxU32 Shader::shaderSwitch = 0;
physx::PxU32 Shader::mMaxBones = 0;

Shader::Shader(const char* vertexShader, const char* fragmentShader, bool isResource, bool errorMessageGui)
{
	mErrorMessageGui = errorMessageGui;
	init(vertexShader, NULL, GL_TRIANGLES, GL_TRIANGLES, 0, fragmentShader, isResource);
}

Shader::Shader(const char* vertexShader, const char* geometryShader, GLenum geomInput, GLenum geomOutput, int geomNumVerticesOutput,
			   const char* fragmentShader, bool isResource, bool errorMessageGui)
{
	mErrorMessageGui = errorMessageGui;
	init(vertexShader, geometryShader, geomInput, geomOutput, geomNumVerticesOutput, fragmentShader, isResource);
}



Shader::~Shader()
{
	for (physx::PxU32 curMode = 0; curMode < NUM_SHADER_MODES; curMode++)
	{
		if (mShaderProgram[curMode] != 0 && mVertexShader[curMode] != 0)
			glDetachShader(mShaderProgram[curMode], mVertexShader[curMode]);

		if (mVertexShader[curMode] != 0)
			glDeleteShader(mVertexShader[curMode]);

		if (mGeometryShader[curMode] != 0)
		{
			if (mShaderProgram[curMode] != 0 && mGeometryShader[curMode] != 0)
				glDetachShader(mShaderProgram[curMode], mGeometryShader[curMode]);

			if (mGeometryShader[curMode] != 0)
				glDeleteShader(mGeometryShader[curMode]);
		}
		if (mShaderProgram[curMode] != 0 && mFragmentShader[curMode] != 0)
			glDetachShader(mShaderProgram[curMode], mFragmentShader[curMode]);

		if (mFragmentShader[curMode] != 0)
			glDeleteShader(mFragmentShader[curMode]);

		if (mShaderProgram[curMode] != 0)
			glDeleteProgram(mShaderProgram[curMode]);

		mShaderProgram[curMode] = mVertexShader[curMode] = mGeometryShader[curMode] = mFragmentShader[curMode] = 0;
	}

	free(mVertexShaderSource);
	free(mGeometryShaderSource);
	free(mFragmentShaderSource);
}



bool Shader::compileShaderModes(physx::PxU32* shaderModes, physx::PxU32 numShaderModes)
{
	PX_ASSERT(shaderModes != NULL);
	bool ok = true;
	for (physx::PxU32 i = 0; i < numShaderModes && ok; i++)
	{
		ok &= compileShaderMode(shaderModes[i]);
	}
	return ok;
}



void Shader::activate(physx::PxU32 shaderMode)
{
	if (mGeometryShaderSourceLength > 0)
		shaderMode |= SM_Geometry;

	compileShaderMode(shaderMode);

	if (isValid(shaderMode))
	{
		if (mActiveShader != NULL)
		{
			if (mActiveShader != this || mActiveShader->mCurrentMode != shaderMode)
			{
				mActiveShader->deactivate();
				glUseProgram(mShaderProgram[shaderMode]);
				mCurrentMode = shaderMode;
				mActiveShader = this;
				shaderSwitch++;
			}
		}
		else
		{
			glUseProgram(mShaderProgram[shaderMode]);
			mCurrentMode = shaderMode;
			mActiveShader = this;
			shaderSwitch++;
		}
	}
}



physx::PxU32 Shader::deactivate()
{
	physx::PxU32 oldMode = mCurrentMode;

	GLint curProg;
	glGetIntegerv(GL_CURRENT_PROGRAM, &curProg);
	PX_ASSERT(curProg == mShaderProgram[mCurrentMode]);

	PX_ASSERT(isValid(mCurrentMode));
	for (tAttributes::iterator it = mAttributes[mCurrentMode].begin(); it != mAttributes[mCurrentMode].end(); ++it)
	{
		glDisableVertexAttribArray(it->second.index);
	}
	if (mGlShaderAttributes[mCurrentMode] & gl_VERTEX)
		glDisableClientState(GL_VERTEX_ARRAY);

	if (mGlShaderAttributes[mCurrentMode] & gl_NORMAL)
		glDisableClientState(GL_NORMAL_ARRAY);

	if (mGlShaderAttributes[mCurrentMode] & gl_COLOR)
		glDisableClientState(GL_COLOR_ARRAY);

	if (mGlShaderAttributes[mCurrentMode] & gl_TEXTURE)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glUseProgram(0);
	mCurrentMode = NUM_SHADER_MODES;
	mActiveShader = NULL;

	return oldMode;
}



bool Shader::addMode( Shader_Mode mode )
{
	if ((mCurrentMode & mode) == 0)
	{
		physx::PxU32 newMode = mCurrentMode | mode;
		deactivate();
		activate(newMode);
		return true;
	}
	return false;
}



void Shader::removeMode(Shader_Mode mode)
{
	if ((mCurrentMode & mode) == mode)
	{
		physx::PxU32 newMode = mCurrentMode & ~mode;
		deactivate();
		activate(newMode);
	}
}


physx::PxU32 Shader::getMaxBones()
{
	if (mMaxBones == 0)
	{
		GLint params[4];
		params[0] = 0;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, params);

#if 0	// PH: This does not seem to work properly on pre GF8 cards
		mMaxBones = params[0];
		mMaxBones -= 6 * 4; // 6 more registers used for other uniforms...

		// PH: do not use more than 100 (only tested value so far on GF8800 and higher)
		// HL: we have a sample with more than 100
		mMaxBones = physx::PxMin(200u, mMaxBones / 3u);
#else
		if (params[0] <= 1024)
		{
			// pre 8800 cards
			mMaxBones = 80;
		}
		else
		{
			mMaxBones = 200;
		}
#endif
	}
	return mMaxBones;
}



bool Shader::isValid(physx::PxU32 shaderMode)
{
	if (mGeometryShaderSourceLength > 0)
		shaderMode |= SM_Geometry;

	PX_ASSERT(shaderMode < NUM_SHADER_MODES);
	return mShaderProgram[shaderMode] != 0;
}



bool Shader::compileShaderMode(physx::PxU32 shaderMode)
{
	if (mGeometryShaderSourceLength > 0)
	{
		PX_ASSERT(shaderMode & SM_Geometry);
	}

	const physx::PxU32 curMode = shaderMode;

	if (mShaderProgram[curMode] != 0)
		return true;

	char* modeInit[NUM_SHADER_BITS] =
	{
		"#define GPU_SKINNING\n",
		"#define WIREFRAME\n",
		"#define NO_LIGHTING\n",
		"#define WIND\n",
		"#define TEXTURE\n",
		"#define GEOMETRY_STAGE\n",
		"#define PER_VERTEX_COLOR\n",
	};

	char* modeName[NUM_SHADER_BITS] =
	{
		"Gpu Skinning",
		"Wireframe",
		"No Lighting",
		"Wind",
		"Texture",
		"Geometry Stage",
		"Vertex Color",
	};

	bool ok = true;

	mShaderProgram[curMode] = glCreateProgram();
	mVertexShader[curMode] = glCreateShader(GL_VERTEX_SHADER);
	if (mGeometryShaderSourceLength > 0)
		mGeometryShader[curMode] = glCreateShader(GL_GEOMETRY_SHADER_EXT);
	mFragmentShader[curMode] = glCreateShader(GL_FRAGMENT_SHADER);
	mGlShaderAttributes[curMode] = 0;

	const char* source[NUM_SHADER_BITS + 2] = {};
	GLint size[NUM_SHADER_BITS + 2] = {};
	char buf[128];
	sprintf_s(buf, 128, "#version 120\n#define MAX_BONES %d\n", mMaxBones);
	source[0] = buf;
	size[0] = (GLint)strlen(buf);

	physx::PxU32 numActive = 1;
	for (physx::PxU32 i = 0; i < NUM_SHADER_BITS; i++)
	{
		if (curMode & (1 << i))
		{
			source[numActive] = modeInit[i];
			size[numActive] = (GLint)strlen(modeInit[i]);
			numActive++;
		}
	}

	source[numActive] = mVertexShaderSource;
	size[numActive] = mVertexShaderSourceLength;

	glShaderSource(mVertexShader[curMode], numActive+1, source, size);
	glCompileShader(mVertexShader[curMode]);

	if (mGeometryShaderSourceLength > 0)
	{
		source[numActive] = mGeometryShaderSource;
		size[numActive] = mGeometryShaderSourceLength;
		glShaderSource(mGeometryShader[curMode], numActive+1, source, size);
		glCompileShader(mGeometryShader[curMode]);
	}

	source[numActive] = mFragmentShaderSource;
	size[numActive] = mFragmentShaderSourceLength;
	glShaderSource(mFragmentShader[curMode], numActive+1, source, size);
	glCompileShader(mFragmentShader[curMode]);

	// error checking
	int param;
	glGetShaderiv(mVertexShader[curMode], GL_COMPILE_STATUS, &param);
	ok &= param == GL_TRUE;
	printCompileError(mVertexShader[curMode], param == GL_TRUE, mVertexShaderFile);

	if (mGeometryShaderSourceLength > 0)
	{
		glGetShaderiv(mGeometryShader[curMode], GL_COMPILE_STATUS, &param);
		printCompileError(mGeometryShader[curMode], param == GL_TRUE, mGeometryShaderFile);
		ok &= param == GL_TRUE;
	}

	glGetShaderiv(mFragmentShader[curMode], GL_COMPILE_STATUS, &param);
	printCompileError(mFragmentShader[curMode], param == GL_TRUE, mFragmentShaderFile);
	ok &= param == GL_TRUE;

	if (ok)
	{
		glAttachShader(mShaderProgram[curMode], mVertexShader[curMode]);
		if (mGeometryShaderSourceLength > 0)
			glAttachShader(mShaderProgram[curMode], mGeometryShader[curMode]);
		glAttachShader(mShaderProgram[curMode], mFragmentShader[curMode]);

		if (mGeometryShaderSourceLength > 0)
		{
			glProgramParameteriEXT(mShaderProgram[curMode], GL_GEOMETRY_INPUT_TYPE_EXT, mGeometryShaderInputType);
			glProgramParameteriEXT(mShaderProgram[curMode], GL_GEOMETRY_OUTPUT_TYPE_EXT, mGeometryShaderOutputType);

			glProgramParameteriEXT(mShaderProgram[curMode], GL_GEOMETRY_VERTICES_OUT_EXT, mGeometryShaderNumVerticesOutput); 

			int n;
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &n); 
			glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT, &n); 
			n = 0;
		}
		glLinkProgram(mShaderProgram[curMode]);
		glGetProgramiv(mShaderProgram[curMode], GL_LINK_STATUS, &param);
		if (param == GL_TRUE)
		{
			if (DEBUG_SHADERS)
			{
				printf("Shader successfully compiled and linked:\n  Mode = ");
				for (physx::PxU32 i = 0; i < NUM_SHADER_BITS; i++)
				{
					if (curMode & (1 << i))
					{
						printf("%s, ", modeName[i]);
					}
				}
				printf("\n  %s\n  %s\n", mVertexShaderFile, mFragmentShaderFile);
			}

			// Find uniforms
			GLint numUniforms, maxLength;
			glGetProgramiv(mShaderProgram[curMode], GL_ACTIVE_UNIFORMS, &numUniforms);
			glGetProgramiv(mShaderProgram[curMode], GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
			PX_ASSERT(maxLength < 256);
			char buf[256];
			if (DEBUG_SHADERS)
				printf(" Has %d uniforms:\n", numUniforms);

			for (GLint i = 0; i < numUniforms; i++)
			{
				GLint length;
				UniformVariable u;
				glGetActiveUniform(mShaderProgram[curMode], i, 256, &length, &u.size, &u.type, buf);
				PX_ASSERT(length < 256);
				if (strncmp("gl_", buf, 3) != 0)
				{
					u.index = glGetUniformLocation(mShaderProgram[curMode], buf);

					if (DEBUG_SHADERS)
						printf("  Uniform%d %s: Size %d, Type %d\n", i, buf, u.size, u.type);

					mUniforms[curMode][buf] = u;
				}
			}

			// Find attribs
			GLint numAttribs;
			glGetProgramiv(mShaderProgram[curMode], GL_ACTIVE_ATTRIBUTES, &numAttribs);
			glGetProgramiv(mShaderProgram[curMode], GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
			PX_ASSERT(maxLength < 256);
			if (DEBUG_SHADERS)
				printf(" Has %d attributes\n", numAttribs);

			for (GLint i = 0; i < numAttribs; i++)
			{
				GLint length;
				AttributeVariable a;
				glGetActiveAttrib(mShaderProgram[curMode], i, 256, &length, &a.size, &a.type, buf);
				if (strncmp("gl_", buf, 3) != 0)
				{
					a.index = glGetAttribLocation(mShaderProgram[curMode], buf);

					if (DEBUG_SHADERS)
						printf("  Attribute%d %s: Size %d, Type %d\n", a.index, buf, a.size, a.type);

					mAttributes[curMode][buf] = a;
				}
				else
				{
					if (strncmp("gl_Vertex", buf, 9) == 0)
					{						
						mGlShaderAttributes[curMode] |= gl_VERTEX;
					}
					else if (strncmp("gl_Normal", buf, 9) == 0)
					{
						mGlShaderAttributes[curMode] |= gl_NORMAL;
					}
					else if (strncmp("gl_Color", buf, 8) == 0)
					{
						mGlShaderAttributes[curMode] |= gl_COLOR;
					}
					else if (strncmp("gl_MultiTexCoord0", buf, 17) == 0)
					{
						mGlShaderAttributes[curMode] |= gl_TEXTURE;
					}
					else
					{
						printf("Unknown gl attribute: %s\n", buf);
					}
				}
			}
		}
		else
		{
			ok = false;
			printLinkError(mShaderProgram[curMode], ok);
		}
	}
	if (!ok)
	{
		glDetachShader(mShaderProgram[curMode], mVertexShader[curMode]);
		glDeleteShader(mVertexShader[curMode]);

		if (mGeometryShader[curMode] != 0)
		{
			glDetachShader(mShaderProgram[curMode], mGeometryShader[curMode]);
			glDeleteShader(mGeometryShader[curMode]);
		}

		glDetachShader(mShaderProgram[curMode], mFragmentShader[curMode]);
		glDeleteShader(mFragmentShader[curMode]);
		glDeleteProgram(mShaderProgram[curMode]);
		mShaderProgram[curMode] = mVertexShader[curMode] = mGeometryShader[curMode] = mFragmentShader[curMode] = 0;
		printf("Disabling Shader:\n  Mode = ");
		for (physx::PxU32 i = 0; i < NUM_SHADER_BITS; i++)
		{
			if (curMode & (1 << i))
			{
				printf("%s, ", modeName[i]);
			}
		}
		printf("\n  %s\n  %s\n", mVertexShaderFile, mFragmentShaderFile);
	}

	return ok;
}



void Shader::printCompileError(GLuint shader, bool ok, const char* shaderFile)
{
	int infoLogLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

	int sourceLength;
	glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &sourceLength);
	char* source = (char*)malloc(sizeof(char) * sourceLength);
	int slen;
	glGetShaderSource(shader, sourceLength, &slen, source);

	const int errorMessageSize = 10000;
	char errorMessage[errorMessageSize];
	errorMessage[0] = 0;

	if (infoLogLength > 1)
	{
		char* log = (char*)malloc(sizeof(char) * infoLogLength);
		glGetShaderInfoLog(shader, infoLogLength, &slen, log);

		const GLubyte* vendor = glGetString(GL_VENDOR);
		const GLubyte* renderer = glGetString(GL_RENDERER);
		const GLubyte* version = glGetString(GL_VERSION);
		//const GLubyte* extensions = glGetString(GL_EXTENSIONS);
		const char* message = ok ? "Message" : "Compiler Error";
		if (DEBUG_SHADERS || !ok)
		{
			sprintf_s(errorMessage, errorMessageSize, "Shader %s(%s)\nVendor: %s\nRenderer: %s\nVersion: %s\nError:\n%s\nCode:\n%s\n",
				message, shaderFile, vendor, renderer, version, log, source);
		}
		free(log);
	}
	else
	{
		if (DEBUG_SHADERS && false)
		{
			sprintf_s(errorMessage, errorMessageSize, "Code:\n%s\n", source);
		}
	}
	if (errorMessage[0] != 0)
	{
		if (mErrorMessageGui)
		{
			MessageBoxA(NULL, errorMessage, "Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
		}
		else
		{
			printf(errorMessage);
		}
	}
	free(source);
}



void Shader::printLinkError(GLuint shader, bool ok)
{
	int infoLogLength;
	glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

	const int errorMessageSize = 10000;
	char errorMessage[errorMessageSize];
	errorMessage[0] = 0;

	if (infoLogLength > 1)
	{
		char* log = (char*)malloc(sizeof(char) * infoLogLength);
		int slen;
		glGetProgramInfoLog(shader, infoLogLength, &slen, log);

		const GLubyte* vendor = glGetString(GL_VENDOR);
		const GLubyte* renderer = glGetString(GL_RENDERER);
		const GLubyte* version = glGetString(GL_VERSION);
		//const GLubyte* extensions = glGetString(GL_EXTENSIONS);
		const char* message = ok ? "Message" : "Link Error";
		if (DEBUG_SHADERS || !ok)
		{
			sprintf_s(errorMessage, errorMessageSize, "Shader %s\nVendor: %s\nRenderer: %s\nVersion: %s\nError:\n%s\n",
				message, vendor, renderer, version, log);
		}

		free(log);
	}

	if (errorMessage[0] != 0)
	{
		if (mErrorMessageGui)
		{
			MessageBoxA(NULL, errorMessage, "Shader Compile Error", MB_OK | MB_ICONEXCLAMATION);
		}
		else
		{
			printf(errorMessage);
		}
	}
}


bool Shader::setUniform(const char *name, const physx::PxMat33Legacy& value)
{
	GLint uniform = getUniformCommon(name);
	if (uniform != -1)
	{
		float v[9];
		value.getColumnMajor(v);
		glUniformMatrix3fv(uniform, 1, false, v);

		return true;
	}
	return false;
}



bool Shader::setUniform(const char *name, const physx::PxMat34Legacy& value)
{
	GLint uniform = getUniformCommon(name);
	if (uniform != -1)
	{
		float v[16];
		value.getColumnMajor44(v);
		glUniformMatrix4fv(uniform, 1, false, v);

		return true;
	}
	return false;
}



bool Shader::setUniform(const char *name, physx::PxU32 size, const physx::PxVec3* value)
{
	GLint uniform = getUniformCommon(name);
	if (uniform != -1)
	{
		glUniform3fv(uniform, size, (const GLfloat*)value);
		return true;
	}
	return false;
}



bool Shader::setUniform1(const char* name, physx::PxU32 size, const float* value)
{
	GLint uniform = getUniformCommon(name);
	if (uniform != -1)
	{
		glUniform1fv(uniform, size, value);
		return true;
	}
	return false;
}



bool Shader::setUniform3x4(const char *name, physx::PxU32 size, const physx::PxMat34Legacy* value)
{
	GLint uniform = getUniformCommon(name);
	if (uniform != -1)
	{
		glUniform4fv(uniform, size * 3, (const GLfloat*)value);
		return true;
	}
	return false;
}



bool Shader::setUniform4x4(const char *name, physx::PxU32 size, const float* value)
{
	GLint uniform = getUniformCommon(name);
	if (uniform != -1)
	{
		glUniformMatrix4fv(uniform, size, false, value);
		return true;
	}
	return false;
}



bool Shader::setAttribute(const char* name, physx::PxU32 size, physx::PxU32 stride, GLenum type, void* data)
{
	GLint activeProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
	PX_ASSERT(mCurrentMode < NUM_SHADER_MODES);
	PX_ASSERT(activeProgram == mShaderProgram[mCurrentMode]);
	tAttributes::iterator it = mAttributes[mCurrentMode].find(name);
	if (it != mAttributes[mCurrentMode].end())
	{
		physx::PxI32 index = it->second.index;
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, GL_FALSE, stride, data);

		return true;
	}
	return false;
}



void Shader::init(const char* vertexShader, const char* geometryShader, GLenum geomInput, GLenum geomOutput, int geomNumVerticesOutput,
				  const char* fragmentShader, bool isResource)
{
	mCurrentMode = NUM_SHADER_MODES;
	mVertexShaderFile = vertexShader;
	mGeometryShaderFile = geometryShader;
	mFragmentShaderFile = fragmentShader;
	mVertexShaderSource = NULL;
	mVertexShaderSourceLength = 0;
	mGeometryShaderSource = NULL;
	mGeometryShaderSourceLength = 0;
	mGeometryShaderInputType = geomInput;
	mGeometryShaderOutputType = geomOutput;
	mGeometryShaderNumVerticesOutput = geomNumVerticesOutput;
	mFragmentShaderSource = NULL;
	mFragmentShaderSourceLength = 0;

	physx::PxU32 maxBones = getMaxBones();
	for (physx::PxU32 i = 0; i < NUM_SHADER_MODES; i++)
	{
		mShaderProgram[i] = 0;
		mVertexShader[i] = 0;
		mGeometryShader[i] = 0;
		mFragmentShader[i] = 0;
	}
	if (vertexShader == NULL || fragmentShader == NULL)
		return;

	if (geometryShader != NULL && !GLEW_EXT_geometry_shader4)
		return;

	if (!GLEW_VERSION_2_0)
		return;

	loadFile(mVertexShaderFile, isResource, mVertexShaderSource, mVertexShaderSourceLength);
	loadFile(mFragmentShaderFile, isResource, mFragmentShaderSource, mFragmentShaderSourceLength);

	if (mGeometryShaderFile != NULL)
		loadFile(mGeometryShaderFile, isResource, mGeometryShaderSource, mGeometryShaderSourceLength);

	compileShaderMode(mGeometryShaderSourceLength > 0 ? SM_Geometry : 0);
}


void Shader::loadFile(const char* filename, bool isResource, char*& destination, int& destinationLength)
{
	if (destination != NULL)
	{
		PX_ASSERT(destinationLength != 0);
		free(destination);
		destination = NULL;
		destinationLength = 0;
	}
	PX_ASSERT(destinationLength == 0);

#if defined(WIN32) && !_MANAGED
	if (isResource)
	{
		HRSRC handle = FindResourceA(NULL, filename, RT_RCDATA);
		HGLOBAL handle2 = LoadResource(NULL, handle);
		const char* shaderSource = (char*)LockResource(handle2);
		destinationLength = SizeofResource(NULL, handle);

		if (destinationLength > 0)
		{
			destination = (char*)malloc(destinationLength * sizeof(char)+1);
			memcpy(destination, shaderSource, destinationLength);

			destination[destinationLength] = 0;
		}

		UnlockResource(handle2);
	}
	else
#endif
	{
		PX_ASSERT(!isResource);
		FILE* file = NULL;
		fopen_s(&file, filename, "r");
		if (file == NULL)
		{
			printf("Shader file not found\n");
			return;
		}
		// find length
		fseek(file, 0, SEEK_END);
		destinationLength = ftell(file);
		fseek(file, 0, SEEK_SET);

		if (destinationLength > 0)
		{
			destination = (char*)malloc(destinationLength * sizeof(char) + 1);

			int writePosition = 0;
			while (true)
			{
				int character = getc(file);
				if (character == EOF)
					break;

				destination[writePosition++] = (char)character;
			}
			PX_ASSERT(writePosition < destinationLength);
			destination[writePosition] = 0;
		}
		fclose(file);
	}
}



GLint Shader::getUniformCommon(const char* name)
{
	GLint activeProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
	PX_ASSERT(mCurrentMode < NUM_SHADER_MODES);
	PX_ASSERT(activeProgram == mShaderProgram[mCurrentMode]);
	tUniforms::iterator it = mUniforms[mCurrentMode].find(name);
	if (it != mUniforms[mCurrentMode].end())
	{
		PX_ASSERT(it->second.index != -1);
		return it->second.index;
	}
	return -1;
}


#endif