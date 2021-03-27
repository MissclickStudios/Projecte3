#ifndef __ImporterShader__
#define __ImporterShader__

#include <string>
#include "Macros.h"

typedef unsigned int		uint;
typedef unsigned __int32	uint32;
#define VERTEX_SHADER "__Vertex_Shader__"
#define FRAGMENT_SHADER "__Fragment_Shader__"

class R_Shader;

namespace Importer
{
	namespace Shaders
	{
		bool		NULL_API Import(const char* path, R_Shader* shader);
		int			NULL_API ImportVertex(std::string shaderFile, R_Shader* shader);
		int			NULL_API ImportFragment(std::string shaderFile, R_Shader* shader);

		void		NULL_API GetShaderUniforms(R_Shader* shader);
		void		NULL_API SetShaderUniforms(R_Shader* shader);

		uint		NULL_API Save(const R_Shader* shader, char** buffer);
		bool		NULL_API Load(const char* buffer, R_Shader* shader, uint size = 0);
		void		NULL_API Recompile(R_Shader* shader);
	}
}


#endif //__ImporterShader__
