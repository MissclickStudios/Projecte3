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
		bool		MISSCLICK_API Import(const char* path, R_Shader* shader);
		int			ImportVertex(std::string shaderFile, R_Shader* shader);
		int			ImportFragment(std::string shaderFile, R_Shader* shader);

		void		GetShaderUniforms(R_Shader* shader);
		void		SetShaderUniforms(R_Shader* shader);

		uint		Save(const R_Shader* shader, char** buffer);
		bool		Load(const char* buffer, R_Shader* shader, uint size = 0);
		void		Recompile(R_Shader* shader);
	}
}


#endif //__ImporterShader__
