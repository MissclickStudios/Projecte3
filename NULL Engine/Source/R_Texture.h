#ifndef __R_TEXTURE_H__
#define __R_TEXTURE_H__

#include "Resource.h"
#include "TextureSettings.h"

class ParsonNode;

typedef unsigned __int32 uint;

enum class TEXTURE_FORMAT													// The enum values are set according to the values of DevIL's define values/flags.
{
	UNKNOWN			= 0,													// Default format. It means that DevIL could not find the format of the imported texture.
	COLOUR_INDEX	= 0x1900,												// 0x1900 = IL_COLOUR_INDEX. 
	RGB				= 0x1907,												// 0x1907 = IL_RGB.
	RGBA			= 0x1908,												// 0x1908 = IL_RGBA.
	BGR				= 0x80E0,												// 0x80E0 = IL_BGR.
	BGRA			= 0x80E1,												// 0x80E1 = IL_BGRA.
	LUMINANCE		= 0x1909												// 0x1909 = IL_LUMINANCE.
};

struct Texture
{
	Texture();

	uint			id;														// Id of the texture.
	uint			width;													// Width of the texture in pixels.
	uint			height;													// Height of the texture in pixels.
	uint			depth;													// Depth of the texture in pixels.
	uint			bpp;													// Amount of Bytes Per Pixel.
	uint			bytes;													// Size of the texture in bytes.
	TEXTURE_FORMAT	format;													// Colour Index, RGB, RGBA... Adapted to fit the formats that DevIL returns.
	bool			compressed;												// Will be True if the texture comes from a compressed format (DDS, ...).
};

class R_Texture : public Resource
{
public:
	R_Texture();
	~R_Texture();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& meta_root) const override;
	bool LoadMeta(const ParsonNode& meta_root) override;

public:
	Texture			GetTextureData			() const;
	void			SetTextureData			(uint id, uint width, uint height, uint depth, uint bpp, uint bytes, TEXTURE_FORMAT format, bool compressed = true);

	uint			GetTextureID			() const;						// 
	uint			GetTextureWidth			() const;						// 
	uint			GetTextureHeight		() const;						// 
	uint			GetTextureDepth			() const;						// 
	uint			GetTextureBpp			() const;						// 
	uint			GetTextureBytes			() const;						// 
	TEXTURE_FORMAT	GetTextureFormat		() const;						// 
	bool			TextureIsCompressed		() const;						// 

	const char*		GetTextureFormatString	() const;

private:
	Texture			tex_data;
	TextureSettings texture_settings;
};

#endif // !__R_TEXTURE_H__