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
#include <SampleAssetManager.h>
#include <SampleAsset.h>
#include <SampleMaterialAsset.h>
#include <SampleTextureAsset.h>

#include <Renderer.h>

#include <stdio.h>
#include "PxAssert.h"
#include "PsString.h"
#include "PsFile.h"

#define RAPIDXML_NO_EXCEPTIONS
#include <rapidxml.hpp>

namespace rapidxml
{
    void parse_error_handler(const char * /*what*/, void * /*where*/)
    {
		PX_ALWAYS_ASSERT();
    }
}

SampleAssetManager::SampleAssetManager(Renderer &renderer) :
	m_renderer(renderer)
{
}

SampleAssetManager::~SampleAssetManager(void)
{
	PX_ASSERT(m_assets.size() == 0);
	clearSearchPaths();
}

SampleAsset *SampleAssetManager::getAsset(const char *path, SampleAsset::Type type)
{	
	SampleAsset *asset = findAsset(path);
	if(!asset)
	{
		asset = loadAsset(path);
	}
	if(asset && asset->getType() != type)
	{
		releaseAsset(*asset);
		asset = 0;
	}
	if(asset)
	{
		asset->m_numUsers++;
	}
	return asset;
}

void SampleAssetManager::returnAsset(SampleAsset &asset)
{
	PX_ASSERT(asset.m_numUsers > 0);
	if(asset.m_numUsers > 0)
	{
		asset.m_numUsers--;
	}
	if(asset.m_numUsers == 0)
	{
		releaseAsset(asset);
	}
}

void SampleAssetManager::addSearchPath(const char *path)
{
	const physx::PxU32 len = path && *path ? (physx::PxU32)strlen(path) : 0;
	if(len)
	{
		const physx::PxU32 len2 = len+2;
		char *searchPath = new char[len2];
		physx::string::strncpy_s(searchPath, len2, path, len2);
		if(path[len-1] != '/' && path[len-1] != '\\')
		{
			physx::string::strncat_s(searchPath, len2, "/", len2);
		}
		m_searchPaths.push_back(searchPath);
	}
}

void SampleAssetManager::clearSearchPaths()
{
	const physx::PxU32 numSearchPaths = (physx::PxU32)m_searchPaths.size();
	for(physx::PxU32 i=0; i<numSearchPaths; i++)
	{
		delete [] m_searchPaths[i];
	}
	m_searchPaths.clear();
}

SampleAsset *SampleAssetManager::findAsset(const char *path)
{
	SampleAsset *asset = 0;
	physx::PxU32 numAssets = (physx::PxU32)m_assets.size();
	for(physx::PxU32 i=0; i<numAssets; i++)
	{
		if(!strcmp(m_assets[i]->getPath(), path))
		{
			asset = m_assets[i];
			break;
		}
	}
	return asset;
}

static const char *strext(const char *str)
{
	const char *ext = str;
	while(str)
	{
		str = strchr(str, '.');
		if(str)
		{
			str++;
			ext = str;
		}
	}
	return ext;
}

SampleAsset *SampleAssetManager::loadAsset(const char *path)
{
	SampleAsset *asset = 0;
	const char *extension = strext(path);
	if(extension && *extension)
	{
		FILE *file = 0;
		const physx::PxU32 numSearchPaths = (physx::PxU32)m_searchPaths.size();
		for(physx::PxU32 i=0; i<numSearchPaths; i++)
		{
			const char *prefix = m_searchPaths[i];
			char fullPath[512];
			physx::string::strncpy_s(fullPath, 512, prefix, 512);
			physx::string::strncat_s(fullPath, 512, path,   512);
			physx::fopen_s(&file, fullPath, "rb");
			if(file) break;
		}

		if(!file)
			physx::fopen_s(&file, path, "rb");

		PX_ASSERT(file);

		// printf("Path: %s   (%s)\n", path, extension);

		if(file)
		{
			if(!strcmp(extension, "xml"))      asset = loadXMLAsset(*file, path);
			else if(!strcmp(extension, "dds")) asset = loadTextureAsset(*file, path, SampleTextureAsset::DDS);
			else if(!strcmp(extension, "tga")) asset = loadTextureAsset(*file, path, SampleTextureAsset::TGA);
			
			fclose(file);
		}
		else
		{
#define SAM_DEFAULT_MATERIAL "materials/simple_lit.xml"
#define SAM_DEFAULT_TEXTURE "textures/test.dds"

			// report the missing asset
			char msg[1024];

			if( !strcmp(extension, "xml") && strcmp(path, SAM_DEFAULT_MATERIAL) )  // Avoid infinite recursion
			{
				physx::string::sprintf_s(msg, sizeof(msg), "Could not find material: %s, loading default material: %s", 
										 path, SAM_DEFAULT_MATERIAL);
				RENDERER_OUTPUT_MESSAGE(&m_renderer, msg);
				
				return loadAsset(SAM_DEFAULT_MATERIAL);  // Try to use the default asset
			}
			else if(!strcmp(extension, "dds"))
			{
				physx::string::sprintf_s(msg, sizeof(msg), "Could not find texture: %s, loading default texture: %s", 
										 path, SAM_DEFAULT_TEXTURE);
				RENDERER_OUTPUT_MESSAGE(&m_renderer, msg);

				return loadAsset(SAM_DEFAULT_TEXTURE);  // Try to use the default asset
			}
		}
	}
	//PX_ASSERT(asset && asset->isOk());
	if(asset && !asset->isOk())
	{
		delete asset;
		asset = 0;
	}
	if(asset)
	{
		m_assets.push_back(asset);
	}
	return asset;
}

void SampleAssetManager::releaseAsset(SampleAsset &asset)
{
	physx::PxU32 numAssets = (physx::PxU32)m_assets.size();
	physx::PxU32 found     = numAssets;
	for(physx::PxU32 i=0; i<numAssets; i++)
	{
		if(&asset == m_assets[i])
		{
			found = i;
			break;
		}
	}
	PX_ASSERT(found < numAssets);
	if(found < numAssets)
	{
		m_assets[found] = m_assets.back();
		m_assets.pop_back();
		asset.release();
	}
}

SampleAsset *SampleAssetManager::loadXMLAsset(FILE &file, const char *path)
{
	SampleAsset *asset = 0;
	fseek(&file, 0, SEEK_END);
	size_t filelen = ftell(&file);
	fseek(&file, 0, SEEK_SET);
	char *filedata = new char[filelen+1];
	filelen = fread(filedata, 1, filelen, &file);
	filedata[filelen] = 0;
	if(filedata && *filedata)
	{
		rapidxml::xml_document<char>* xmldoc = new rapidxml::xml_document<char>;
		xmldoc->parse<0>(filedata);
		rapidxml::xml_node<char> *rootnode = xmldoc->first_node();
		PX_ASSERT(rootnode);
		if(rootnode)
		{
			const char *name = rootnode->name();
			if(!strcmp(name, "material"))
			{
				asset = new SampleMaterialAsset(*this, *rootnode, path);
			}
		}
		delete xmldoc;
	}
	delete [] filedata;
	return asset;
}

SampleAsset *SampleAssetManager::loadTextureAsset(FILE &file, const char *path, SampleTextureAsset::Type texType)
{
	SampleTextureAsset *asset = 0;
	asset = new SampleTextureAsset(getRenderer(), file, path, texType);
	return asset;
}