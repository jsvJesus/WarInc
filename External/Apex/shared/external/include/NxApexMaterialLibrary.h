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
#ifndef _NX_APEX_MATERIAL_LIBRARY_H_
#define _NX_APEX_MATERIAL_LIBRARY_H_

#include "PsShare.h"
#include "PxVec3.h"

/**
	Texture map types.  Currently the NxApexMaterialLibrary interface only supports one map of each type.
*/
enum NxApexTextureMapType
{
	DIFFUSE_MAP = 0,
	BUMP_MAP,
	NORMAL_MAP,

	TEXTURE_MAP_TYPE_COUNT
};


/**
	Texture pixel format types, a supported by the Destruction tool's bmp, dds, and tga texture loaders.
*/
enum NxApexPixelFormat
{
	PIXEL_FORMAT_UNKNOWN,

	PIXEL_FORMAT_RGB,
	PIXEL_FORMAT_BGR_EXT,
	PIXEL_FORMAT_BGRA_EXT,
	PIXEL_FORMAT_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	PIXEL_FORMAT_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	PIXEL_FORMAT_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};


/**
	Basic interface to query generic texture map data.
*/
class NxApexTextureMap
{
public:

	/** The texture pixel format, supported by the Destruction tool's texture loaders. */
	virtual NxApexPixelFormat	getPixelFormat() const = 0;

	/** The horizontal texture map size, in pixels. */
	virtual physx::PxU32				getWidth() const = 0;

	/** The vertical texture map size, in pixels. */
	virtual physx::PxU32				getHeight() const = 0;

	/** The number of color/alpha/etc. channels. */
	virtual physx::PxU32				getComponentCount() const = 0;

	/** The size, in bytes, of the pixel buffer. */
	virtual physx::PxU32				getPixelBufferSize() const = 0;

	/** The beginning address of the pixel buffer. */
	virtual physx::PxU8*				getPixels() const = 0;
};


/**
	Basic interface to query generic render material data.
*/
class NxApexMaterial
{
public:

	/** The material's name, used by the named resource provider. */
	virtual const char*			getName() const = 0;

	/** Access to the material's texture maps, indexed by NxApexTextureMapType. */
	virtual NxApexTextureMap*	getTextureMap( NxApexTextureMapType type ) const = 0;

	/** The ambient lighting color of the material. */
	virtual const physx::PxVec3&		getAmbient() const = 0;

	/** The diffuse lighting color of the material. */
	virtual const physx::PxVec3&		getDiffuse() const = 0;

	/** The specular lighting color of the material. */
	virtual const physx::PxVec3&		getSpecular() const = 0;

	/** The opacity of the material. */
	virtual physx::PxF32				getAlpha() const = 0;

	/** The shininess (specular power) of the material. */
	virtual physx::PxF32				getShininess() const = 0;
};


/**
	Material library skeleton interface.
*/
class NxApexMaterialLibrary
{
public:

	/** Saves the material to an physx::PxFileBuf. */
	virtual void			serialize( physx::PxFileBuf& stream ) const = 0;

	/** Loads material from an physx::PxFileBuf. */
	virtual void			deserialize( physx::PxFileBuf& stream ) = 0;

	/**
		Query a material by name.
		If the material already exists, it is returned and 'created' is set to false. 
		If the material did not exist before, it is created, returned, and 'created' is set to true.
	*/
	virtual NxApexMaterial*	getMaterial( const char* materialName, bool& created ) = 0;

	/**
		Remove and delete named material.
		Returns true if the material was found, false if it was not.
	*/
	virtual bool			deleteMaterial( const char* materialName ) = 0;
};


#endif // #ifndef _NX_APEX_MATERIAL_LIBRARY_H_
