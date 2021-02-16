#ifndef __I_TEXTURES_H__
#define __I_TEXTURES_H__

class R_Texture;

typedef unsigned int		uint;

namespace Importer
{
	namespace Textures
	{	
		void	Init	();
		void	CleanUp	();
		
		bool	Import	(const char* buffer, uint size, R_Texture* r_texture);
		uint	Save	(const R_Texture* r_texture, char** buffer);
		bool	Load	(const char* buffer, const uint size, R_Texture* r_texture);

		namespace Utilities
		{	
			uint CreateTexture	(const void* data, uint width,
													uint height,
													uint target = 0x0DE1, 				// 0x0DE1 = GL_TEXTURE_2D
													int filter = 0x2600,				// 0x2600 = GL_NEAREST
													int wrapping = 0x2901,				// 0x2901 = GL_REPEAT
													int internal_format = 0x1908,		// 0x1908 = GL_RGBA
													uint format = 0x1908); 				// 0x1908 = GL_RGBA
		}
	}
}

#endif // !__I_TEXTURES_H__