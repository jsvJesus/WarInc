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
#include <SampleTextureAsset.h>

#include <Renderer.h>
#include <RendererTexture2D.h>
#include <RendererTexture2DDesc.h>

#include "nv_dds.h"

#ifdef RENDERER_ENABLE_TGA_SUPPORT
# include "targa.h"
#endif

SampleTextureAsset::SampleTextureAsset(Renderer &renderer, FILE &file, const char *path, Type texType) :
	SampleAsset(ASSET_TEXTURE, path)
{
	m_texture = 0;
	
	switch(texType)
	{
	case DDS: loadDDS(renderer, file); break;
	case TGA: loadTGA(renderer, file); break;
	default: PX_ASSERT(0 && "Invalid texture type requested"); break;
	}
}

SampleTextureAsset::~SampleTextureAsset(void)
{
	if(m_texture) m_texture->release();
}

void SampleTextureAsset::loadDDS(Renderer &renderer, FILE &file) 
{
	nv_dds::CDDSImage ddsimage;
	bool ok = ddsimage.load(&file, false);
	PX_ASSERT(ok);
	if(ok)
	{
		RendererTexture2DDesc tdesc;
		nv_dds::TextureFormat ddsformat = ddsimage.get_format();
		switch(ddsformat)
		{
			case nv_dds::TextureBGRA:      tdesc.format = RendererTexture2D::FORMAT_B8G8R8A8; break;
			case nv_dds::TextureDXT1:      tdesc.format = RendererTexture2D::FORMAT_DXT1;     break;
			case nv_dds::TextureDXT3:      tdesc.format = RendererTexture2D::FORMAT_DXT3;     break;
			case nv_dds::TextureDXT5:      tdesc.format = RendererTexture2D::FORMAT_DXT5;     break;
		}
		tdesc.width     = ddsimage.get_width();
		tdesc.height    = ddsimage.get_height();
		// if there is 1 mipmap, nv_dds reports 0
		tdesc.numLevels = ddsimage.get_num_mipmaps()+1;
		PX_ASSERT(tdesc.isValid());
		m_texture = renderer.createTexture2D(tdesc);
		PX_ASSERT(m_texture);
		if(m_texture)
		{
			physx::PxU32 pitch  = 0;
			void *buffer = m_texture->lockLevel(0, pitch);
			PX_ASSERT(buffer);
			if(buffer)
			{
				//physx::PxU32 size = ddsimage.get_size();
				
				physx::PxU8       *levelDst    = (physx::PxU8*)buffer;
				const physx::PxU8 *levelSrc    = (physx::PxU8*)(unsigned char*)ddsimage;
				const physx::PxU32 levelWidth  = m_texture->getWidthInBlocks();
				const physx::PxU32 levelHeight = m_texture->getHeightInBlocks();
				const physx::PxU32 rowSrcSize  = levelWidth * m_texture->getBlockSize();
				PX_ASSERT(rowSrcSize <= pitch); // the pitch can't be less than the source row size.
				for(physx::PxU32 row=0; row<levelHeight; row++)
				{
					memcpy(levelDst, levelSrc, rowSrcSize);
					levelDst += pitch;
					levelSrc += rowSrcSize;
				}
			}
			m_texture->unlockLevel(0);

			for(physx::PxU32 i=1; i<tdesc.numLevels; i++)
			{
				void *buffer = m_texture->lockLevel(i, pitch);
				PX_ASSERT(buffer);
				if(buffer && pitch )
				{
					const nv_dds::CSurface &surface = ddsimage.get_mipmap(i-1);
					//physx::PxU32 size = surface.get_size();
					
					physx::PxU8       *levelDst    = (physx::PxU8*)buffer;
					const physx::PxU8 *levelSrc    = (physx::PxU8*)(unsigned char*)surface;
					const physx::PxU32 levelWidth  = RendererTexture2D::getFormatNumBlocks(surface.get_width(),  m_texture->getFormat());
					const physx::PxU32 levelHeight = RendererTexture2D::getFormatNumBlocks(surface.get_height(), m_texture->getFormat());
					const physx::PxU32 rowSrcSize  = levelWidth * m_texture->getBlockSize();
					PX_ASSERT(rowSrcSize <= pitch); // the pitch can't be less than the source row size.
					for(physx::PxU32 row=0; row<levelHeight; row++)
					{
						memcpy(levelDst, levelSrc, rowSrcSize);
						levelDst += pitch;
						levelSrc += rowSrcSize;
					}
				}
				m_texture->unlockLevel(i);
			}
		}
	}
}

void SampleTextureAsset::loadTGA(Renderer &renderer, FILE &file)
{
#ifdef RENDERER_ENABLE_TGA_SUPPORT

	tga_image* image = new tga_image();
	bool ok = (TGA_NOERR == tga_read_from_FILE( image, &file ));
	
	// flip it to make it look correct in the SampleFramework's renderer
	tga_flip_vert( image );

	PX_ASSERT(ok);
	if( ok )
	{
		RendererTexture2DDesc tdesc;
		int componentCount = image->pixel_depth/8;
		if( componentCount == 3 || componentCount == 4 )
		{
			tdesc.format = RendererTexture2D::FORMAT_B8G8R8A8;

			tdesc.width     = image->width;
			tdesc.height    = image->height;

			tdesc.numLevels = 1;
			PX_ASSERT(tdesc.isValid());
			m_texture = renderer.createTexture2D(tdesc);
			PX_ASSERT(m_texture);

			if(m_texture)
			{
				physx::PxU32 pitch  = 0;
				void *buffer = m_texture->lockLevel(0, pitch);
				PX_ASSERT(buffer);
				if(buffer)
				{
					physx::PxU8       *levelDst    = (physx::PxU8*)buffer;
					const physx::PxU8 *levelSrc    = (physx::PxU8*)image->image_data;
					const physx::PxU32 levelWidth  = m_texture->getWidthInBlocks();
					const physx::PxU32 levelHeight = m_texture->getHeightInBlocks();
					const physx::PxU32 rowSrcSize  = levelWidth * m_texture->getBlockSize();
					PX_ASSERT(rowSrcSize <= pitch); // the pitch can't be less than the source row size.
					for(physx::PxU32 row=0; row<levelHeight; row++)
					{ 
						if( componentCount == 3 )
						{
							// copy per pixel to handle RBG case, based on component count
							for(physx::PxU32 col=0; col<levelWidth; col++)
							{
								*levelDst++ = levelSrc[0];
								*levelDst++ = levelSrc[1];
								*levelDst++ = levelSrc[2];
								*levelDst++ = 0xFF; //alpha
								levelSrc += componentCount;
							}
						}
						else
						{
							memcpy(levelDst, levelSrc, rowSrcSize);
							levelDst += pitch;
							levelSrc += rowSrcSize;
						}
					}
				}
				m_texture->unlockLevel(0);
			}
		}
	}
	delete image;

#endif /* RENDERER_ENABLE_TGA_SUPPORT */
}

RendererTexture2D *SampleTextureAsset::getTexture(void)
{
	return m_texture;
}

bool SampleTextureAsset::isOk(void) const
{
	return m_texture ? true : false;
}
