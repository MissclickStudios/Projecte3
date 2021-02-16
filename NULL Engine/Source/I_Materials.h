#ifndef __I_MATERIALS_H__
#define __I_MATERIALS_H__

struct aiScene;
struct aiMaterial;

class R_Material;
class R_Texture;

typedef unsigned int uint;

namespace Importer
{
	namespace Materials																	// Importing, saving and loading aiMaterials with Assimp.
	{
		void Import	(const aiMaterial* assimpMaterial, R_Material* rMaterial);			//Processes an aiMaterial into a ready-to-use R_Material. ERROR = nullptr.
		uint Save	(const R_Material* rMaterial, char** buffer);						// Processes R_Material data into a ready-to-save buffer. Returns buffer size (0 = ERROR).
		bool Load	(const char* buffer, R_Material* rMaterial);						// Processes buffer data into a ready-to-use R_Material. Returns nullptr on error.
	}
}

#endif // !__I_MATERIALS_H__