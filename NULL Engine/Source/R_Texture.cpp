#include "OpenGL.h"

#include "VariableTypedefs.h"

#include "JSONParser.h"

#include "R_Texture.h"

R_Texture::R_Texture() : Resource(RESOURCE_TYPE::TEXTURE)
{

}

R_Texture::~R_Texture()
{

}

bool R_Texture::CleanUp()
{
	bool ret = true;

	glDeleteTextures(1, (GLuint*)&texData.id);

	return ret;
}

bool R_Texture::SaveMeta(ParsonNode& metaRoot) const
{
	bool ret = true;

	ParsonArray containedArray = metaRoot.SetArray("ContainedResources");

	ParsonNode settings = metaRoot.SetNode("ImportSettings");
	textureSettings.Save(settings);

	return ret;
}

bool R_Texture::LoadMeta(const ParsonNode& metaRoot)
{
	bool ret = true;



	return ret;
}


// --- R_TEXTURE METHODS ---
Texture R_Texture::GetTextureData() const
{
	return texData;
}

void R_Texture::SetTextureData(uint id, uint width, uint height, uint depth, uint bpp, uint bytes, TEXTURE_FORMAT format, bool compressed)
{
	texData.id				= id;
	texData.width			= width;
	texData.height			= height;
	texData.depth			= depth;
	texData.bpp			= bpp;
	texData.bytes			= bytes;
	texData.format			= format;
	texData.compressed		= compressed;
}

uint R_Texture::GetTextureID() const
{
	return texData.id;
}

uint R_Texture::GetTextureWidth() const
{
	return texData.width;
}

uint R_Texture::GetTextureHeight() const
{
	return texData.height;
}

uint R_Texture::GetTextureDepth() const
{
	return texData.depth;
}

uint R_Texture::GetTextureBpp() const
{
	return texData.bpp;
}

uint R_Texture::GetTextureBytes() const
{
	return texData.bytes;
}

TEXTURE_FORMAT R_Texture::GetTextureFormat() const
{
	return texData.format;
}

bool R_Texture::TextureIsCompressed() const
{
	return texData.compressed;
}

const char* R_Texture::GetTextureFormatString() const
{
	switch (texData.format)
	{
	case TEXTURE_FORMAT::UNKNOWN:		{ return "UNKNOWN"; }		break;
	case TEXTURE_FORMAT::COLOUR_INDEX:	{ return "COLOUR_INDEX"; }	break;
	case TEXTURE_FORMAT::RGB:			{ return "RGB"; }			break;
	case TEXTURE_FORMAT::RGBA:			{ return "RGBA"; }			break;
	case TEXTURE_FORMAT::BGR:			{ return "BGR"; }			break;
	case TEXTURE_FORMAT::BGRA:			{ return "BGRA"; }			break;
	case TEXTURE_FORMAT::LUMINANCE:		{ return "LUMINANCE"; }		break;
	}

	return "NONE";
}

// --- TEXTURE STRUCT METHODS ---
Texture::Texture()
{
	id			= 0;
	width		= 0;
	height		= 0;
	depth		= 0;
	bpp			= 0;
	bytes		= 0;
	format		= TEXTURE_FORMAT::UNKNOWN;
	compressed	= true;
}