#ifndef __I_SCENES_H__
#define __I_SCENES_H__

#include <vector>
#include <map>

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiAnimation;

struct ModelNode;
struct MaterialData;

class Resource;
class R_Model;

typedef unsigned int uint;

namespace Importer
{
	namespace Scenes
	{
		void Import(const char* buffer, uint size, R_Model* rModel);
		uint Save(const R_Model* rModel, char** buffer);
		bool Load(const char* buffer, R_Model* rModel);

		namespace Utilities
		{
			void ProcessNode(const aiScene* assimpScene, const aiNode* assimpNode, R_Model* rModel, const ModelNode& parent);
			
			const aiNode* ImportTransform(const aiNode* assimpNode, ModelNode& modelNode);
			void ImportMeshesAndMaterials(const aiScene* assimpScene, const aiNode* assimpNode, R_Model* rModel, ModelNode& modelNode);
				 
			void ImportMesh(const char* nodeName, const aiMesh* assimpMesh, ModelNode& modelNode);
			void ImportMaterial(const char* nodeName, const aiMaterial* assimpMaterial, R_Model* rModel, ModelNode& modelNode);
			void ImportTexture(const std::vector<MaterialData>& materials, ModelNode& modelNode);
			void ImportShader(const char* nodeName, R_Model* rModel, ModelNode& modelNode);
				 
			void ImportAnimations(const aiScene* assimpScene, R_Model* modelNode);
				 
			bool NodeIsDummyNode(const aiNode& assimpNode);
				 
			void CheckAndApplyForcedUID(Resource* resource);

			static std::vector<aiMesh*>				aiMeshes;
			static std::vector<aiMaterial*>			aiMaterials;
			static std::map<uint, ModelNode>		loadedNodes;
			static std::map<std::string, uint32>	loadedTextures;

			static std::map<std::string, uint32>	forcedUIDs;								// First item is name of the model node and second the UID to force the resource to have.
		}
	}
}

#endif // !__I_SCENES_H__