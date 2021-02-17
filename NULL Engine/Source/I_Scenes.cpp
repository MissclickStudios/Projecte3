// ----------------------------------------------------
// I_Scenes.cpp --- Importing .FBX files into scenes.
// Interface class between the Engine and Assimp.
// ----------------------------------------------------

#include <string>

#include "Assimp.h"
#include "JSONParser.h"

#include "VariableTypedefs.h"
#include "Log.h"

#include "Random.h"

#include "Application.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"

#include "Resource.h"																					// See if these includes can be delegated to the other importers.
#include "R_Model.h"																					// 
#include "R_Mesh.h"																						// 
#include "R_Material.h"																					// 
#include "R_Texture.h"																					//
#include "R_Animation.h"																				// --------------------------------------------------------------

#include "I_Meshes.h"
#include "I_Materials.h"
#include "I_Textures.h"
#include "I_Animations.h"

#include "I_Scenes.h"

#include "MemoryManager.h"

#pragma comment (lib, "Source/Dependencies/Assimp/libx86/assimp.lib")

using namespace Importer::Scenes;																		// Not a good thing to do but it will be employed sparsely and only inside this .cpp

void Importer::Scenes::Import(const char* buffer, uint size, R_Model* rModel)
{
	if (rModel == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Model! Error: R_Model* was nullptr.");
		return;
	}
	
	std::string errorString = "[ERROR] Importer: Could not Import Model { " + std::string(rModel->GetAssetsFile()) + " }";
	
	if (buffer == nullptr)
	{
		LOG("%s! Error: Buffer was nullptr.", errorString.c_str());
		return;
	}
	if (size == 0)
	{
		LOG("%s! Error: Size was 0.", errorString.c_str());
		return;
	}
	
	LOG("[STATUS] Importing Scene: %s", rModel->GetAssetsFile());

	const aiScene* assimpScene = aiImportFileFromMemory(buffer, size, aiProcessPreset_TargetRealtime_MaxQuality, nullptr);

	if (assimpScene == nullptr || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
	{
		LOG("%s! Error: Assimp Error [%s]", errorString.c_str(), aiGetErrorString());
		return;
	}

	for (uint i = 0; i < assimpScene->mNumMeshes; ++i)
	{
		Utilities::aiMeshes.push_back(assimpScene->mMeshes[i]);
	
		uint matIndex = assimpScene->mMeshes[i]->mMaterialIndex;
		if (matIndex >= 0)
		{
			Utilities::aiMaterials.push_back(assimpScene->mMaterials[matIndex]);
		}
	}

	Utilities::ProcessNode(assimpScene, assimpScene->mRootNode, rModel, ModelNode());							// First Parent is empty. Later assigned to scene_root.

	Utilities::ImportAnimations(assimpScene, rModel);

	Utilities::aiMeshes.clear();
	Utilities::aiMaterials.clear();
	Utilities::loadedNodes.clear();
	Utilities::loadedTextures.clear();
}

void Importer::Scenes::Utilities::ProcessNode(const aiScene* assimpScene, const aiNode* assimpNode, R_Model* rModel, const ModelNode& parent)
{
	ModelNode modelNode	= ModelNode();
	modelNode.uid			= Random::LCG::GetRandomUint();
	modelNode.parentUID	= parent.uid;

	assimpNode = Utilities::ImportTransform(assimpNode, modelNode);
	Utilities::ImportMeshesAndMaterials(assimpScene, assimpNode, rModel, modelNode);

	modelNode.name	= (assimpNode == assimpScene->mRootNode) ? rModel->GetAssetsFile() : assimpNode->mName.C_Str();

	rModel->modelNodes.push_back(modelNode);

	for (uint i = 0; i < assimpNode->mNumChildren; ++i)
	{
		ProcessNode(assimpScene, assimpNode->mChildren[i], rModel, modelNode);
	}
}

const aiNode* Importer::Scenes::Utilities::ImportTransform(const aiNode* assimpNode, ModelNode& model_node)
{
	// Assimp generates dummy nodes to store multiple FBX transformations.
	// All those transformations will be collapsed to the first non-dummy node.
	
	aiTransform aiT;																						// Transform structure for Assimp. aiVector3D and aiQuaternion.
	Transform	maT;																						// Transform structure for MathGeoLib. float3 and Quat.

	assimpNode->mTransformation.Decompose(aiT.scale, aiT.rotation, aiT.position);							// --- Getting the Transform stored in the node.

	maT.position	= { aiT.position.x, aiT.position.y, aiT.position.z };								// 
	maT.rotation	= { aiT.rotation.x, aiT.rotation.y, aiT.rotation.z, aiT.rotation.w };				// 
	maT.scale		= { aiT.scale.x, aiT.scale.y, aiT.scale.z };											// ---------------------------------------------

	while (NodeIsDummyNode(*assimpNode))																		// All dummy nodes will contain the "_$AssimpFbx$_" string and only one child node.
	{
		assimpNode = assimpNode->mChildren[0];																	// As dummies will only have one child, selecting the next one to process is easy.

		assimpNode->mTransformation.Decompose(aiT.scale, aiT.rotation, aiT.position);						// --- Getting the Transform stored in the dummy node.

		Transform dummy;																					// 
		dummy.position	= { aiT.position.x, aiT.position.y, aiT.position.z };							// 
		dummy.rotation	= { aiT.rotation.x, aiT.rotation.y, aiT.rotation.z, aiT.rotation.w };			// 
		dummy.scale		= { aiT.scale.x, aiT.scale.y, aiT.scale.z };										// ---------------------------------------------------

		/*ma_t.position	= */maT.position.Add(dummy.position);												// --- Adding the dummy's Transform to the current one.
		/*ma_t.rotation	= */maT.rotation.Mul(dummy.rotation);												// 
		/*ma_t.scale	= */maT.scale.Mul(dummy.scale);													// ----------------------------------------------------
	}
	
	model_node.transform	= maT;

	LOG("[IMPORTER] Imported the transforms of node: %s", assimpNode->mName.C_Str());

	return assimpNode;
}

void Importer::Scenes::Utilities::ImportMeshesAndMaterials(const aiScene* assimpScene, const aiNode* assimpNode, R_Model* rModel, ModelNode& modelNode)
{
	if (assimpScene == nullptr || assimpNode == nullptr || rModel == nullptr)
	{
		return;
	}
	if (!assimpScene->HasMeshes())
	{
		return;
	}
	
	const char* nodeName = assimpNode->mName.C_Str();

	for (uint i = 0; i < assimpNode->mNumMeshes; ++i)
	{	
		std::map<uint, ModelNode>::iterator item = loadedNodes.find(assimpNode->mMeshes[i]);
		if (item != loadedNodes.end())
		{
			modelNode.meshUID		= item->second.meshUID;
			modelNode.materialUID = item->second.materialUID;
			modelNode.textureUID	= item->second.textureUID;
			continue;
		}
		
		aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];

		if (assimpMesh != nullptr && assimpMesh->HasFaces())
		{
			Importer::Scenes::Utilities::ImportMesh(nodeName, assimpMesh, modelNode);

			if (assimpMesh->mMaterialIndex >= 0)
			{
				aiMaterial* assimpMaterial = assimpScene->mMaterials[assimpMesh->mMaterialIndex];

				Importer::Scenes::Utilities::ImportMaterial(nodeName, assimpMaterial, rModel, modelNode);
			}
		}

		loadedNodes.emplace(assimpNode->mMeshes[i], modelNode);
	}
}

void Importer::Scenes::Utilities::ImportMesh(const char* nodeName, const aiMesh* assimpMesh, ModelNode& modelNode)
{
	std::string assetsPath = ASSETS_MODELS_PATH + std::string(nodeName) + MESHES_EXTENSION;						// As meshes are contained in models, the assets path is kind of made-up.
	R_Mesh* rMesh = (R_Mesh*)App->resourceManager->CreateResource(ResourceType::MESH, assetsPath.c_str());

	Importer::Meshes::Import(assimpMesh, rMesh);

	if (rMesh == nullptr)
	{
		return;
	}
	
	modelNode.meshUID = rMesh->GetUID();
	App->resourceManager->SaveResourceToLibrary(rMesh);
	App->resourceManager->DeallocateResource(rMesh);
}

void Importer::Scenes::Utilities::ImportMaterial(const char* nodeName, const aiMaterial* assimpMaterial, R_Model* rModel, ModelNode& modelNode)
{
	std::string matFullPath	= App->fileSystem->GetDirectory(rModel->GetAssetsPath()) + nodeName + MATERIALS_EXTENSION;
	R_Material* rMaterial		= (R_Material*)App->resourceManager->CreateResource(ResourceType::MATERIAL, matFullPath.c_str());			// Only considering one texture per mesh.

	if (rMaterial == nullptr)
	{
		return;
	}
	
	Importer::Materials::Import(assimpMaterial, rMaterial);
	
	modelNode.materialUID = rMaterial->GetUID();																								//

	Utilities::ImportTexture(rMaterial->materials, modelNode);
	
	App->resourceManager->SaveResourceToLibrary(rMaterial);
	App->resourceManager->DeallocateResource(rMaterial);
}

void Importer::Scenes::Utilities::ImportTexture(const std::vector<MaterialData>& materials, ModelNode& modelNode)
{
	for (uint i = 0; i < materials.size(); ++i)
	{
		const char* texPath	= materials[i].textureAssetsPath.c_str();
		char* buffer			= nullptr;
		uint read				= App->fileSystem->Load(texPath, &buffer);
		if (buffer != nullptr && read > 0)
		{
			std::map<std::string, uint32>::iterator item = loadedTextures.find(texPath);
			if (item != loadedTextures.end())
			{
				if (materials[i].type == TextureType::DIFFUSE)
				{
					modelNode.textureUID = item->second;
					modelNode.textureName = App->fileSystem->GetFileAndExtension(texPath);
				}

				RELEASE_ARRAY(buffer);
				continue;
			}
			
			R_Texture* rTexture = (R_Texture*)App->resourceManager->CreateResource(ResourceType::TEXTURE, texPath);
			uint texId = Importer::Textures::Import(buffer, read, rTexture);											//

			if (texId == 0)
			{
				App->resourceManager->DeleteResource(rTexture);
				RELEASE_ARRAY(buffer);
				continue;
			}

			if (materials[i].type == TextureType::DIFFUSE)																// For now only the diffuse texture will be used on models' meshes.
			{
				modelNode.textureUID	= rTexture->GetUID();
				modelNode.textureName = rTexture->GetAssetsFile();
			}

			loadedTextures.emplace(texPath, rTexture->GetUID());

			App->resourceManager->SaveResourceToLibrary(rTexture);
			App->resourceManager->DeallocateResource(rTexture);

			RELEASE_ARRAY(buffer);
		}
		else
		{
			LOG("[ERROR] Importer: Could not load texture from given path! Path: %s", texPath);
		}
	}
}

void Importer::Scenes::Utilities::ImportAnimations(const aiScene* assimpScene, R_Model* rModel)
{
	if (assimpScene == nullptr)
	{
		LOG("[ERROR] Scene Importer: Could not Import Model's Animations! Error: Given aiScene* was nullptr");
		return;
	}
	if (!assimpScene->HasAnimations())
	{
		LOG("[WARNING] Scene Importer: Model had no animations to import.");
		return;
	}

	for (uint i = 0; i < assimpScene->mNumAnimations; ++i)
	{
		aiAnimation* assimpAnimation = assimpScene->mAnimations[i];

		std::string name			= assimpAnimation->mName.C_Str();
		std::string assetsPath		= ASSETS_MODELS_PATH + name + ANIMATIONS_EXTENSION;
		R_Animation* rAnimation	= (R_Animation*)App->resourceManager->CreateResource(ResourceType::ANIMATION, assetsPath.c_str());

		if (rAnimation == nullptr)
		{
			continue;
		}

		Importer::Animations::Import(assimpAnimation, rAnimation);

		rModel->animations.emplace(rAnimation->GetUID(), rAnimation->GetName());

		App->resourceManager->SaveResourceToLibrary(rAnimation);
		App->resourceManager->DeallocateResource(rAnimation);
	}
}

bool Importer::Scenes::Utilities::NodeIsDummyNode(const aiNode& assimpNode)
{
	return (strstr(assimpNode.mName.C_Str(), "_$AssimpFbx$_") != nullptr && assimpNode.mNumChildren == 1);	// All dummy nodes will contain the "_$AssimpFbx$_" string and only one child node.
}

uint Importer::Scenes::Save(const R_Model* rModel, char** buffer)
{	
	uint written = 0;

	if (rModel == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save R_Model* in Library! Error: Given R_Model* was nullptr.");
		return 0;
	}

	std::string errorString = "[ERROR] Importer: Could not Save Model { " + std::string(rModel->GetAssetsFile()) + " } from Library";

	ParsonNode rootNode			= ParsonNode();																						// --- GENERATING THE REQUIRED PARSON NODE AND ARRAY
	ParsonArray modelNodesArray	= rootNode.SetArray("ModelNodes");																	// -------------------------------------------------
	ParsonArray animationsArray	= rootNode.SetArray("Animations");

	for (uint i = 0; i < rModel->modelNodes.size(); ++i)																				// --- SAVING MODEL NODE DATA
	{
		ParsonNode modelNode = modelNodesArray.SetNode(rModel->modelNodes[i].name.c_str());
		rModel->modelNodes[i].Save(modelNode);
	}

	std::map<uint32, std::string>::const_iterator item;
	for (item = rModel->animations.cbegin(); item != rModel->animations.cend(); ++item)
	{
		ParsonNode animationNode = animationsArray.SetNode(item->second.c_str());
		animationNode.SetNumber("UID", (double)item->first);
		animationNode.SetString("Name", item->second.c_str());
	}

	std::string path	= MODELS_PATH + std::to_string(rModel->GetUID()) + MODELS_EXTENSION;
	written				= rootNode.SerializeToFile(path.c_str(), buffer);
	if (written > 0)
	{
		LOG("[STATUS] Importer: Successfully saved Model { %s } in Library! Path: %s", rModel->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("%s! Error: File System could not write the file.", errorString.c_str());
	}

	return written;
}

bool Importer::Scenes::Load(const char* buffer, R_Model* rModel)
{
	bool ret = true;

	if (rModel == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Model from Library! Error: R_Model was nullptr.");
		return false;
	}
	
	std::string errorString = "[ERROR] Importer: Could not Load Model { " + std::string(rModel->GetAssetsFile()) + " } from Library";

	if (buffer == nullptr)
	{
		LOG("%s! Error: Given buffer was nullptr.", errorString.c_str());
		return false;
	}

	ParsonNode rootNode			= ParsonNode(buffer);
	ParsonArray modelNodesArray	= rootNode.GetArray("ModelNodes");
	ParsonArray animationsArray	= rootNode.GetArray("Animations");
	if (!rootNode.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Root Node from the passed buffer.", errorString.c_str());
		return false;
	}
	if (!modelNodesArray.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the ModelNodes Array from the Root Node.", errorString.c_str());
		return false;
	}
	if (!animationsArray.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the Animations Array from the Root Node.", errorString.c_str());
	}

	for (uint i = 0; i < modelNodesArray.size; ++i)
	{
		ParsonNode parsonNode = modelNodesArray.GetNode(i);
		if (!parsonNode.NodeIsValid())
		{
			LOG("%s! Error: Could not parse Node %s from Model Nodes Array.", errorString.c_str(), i);
			return false;
		}

		ModelNode modelNode = ModelNode();
		modelNode.Load(parsonNode);

		rModel->modelNodes.push_back(modelNode);
	}

	for (uint i = 0; i < animationsArray.size; ++i)
	{
		ParsonNode animationsNode = animationsArray.GetNode(i);
		if (!animationsNode.NodeIsValid())
		{
			LOG("%s! Error: Could not parse Node %s from Animations Array.", errorString.c_str());
			return false;
		}

		uint32 animationUid		= (uint32)animationsNode.GetNumber("UID");
		std::string animationName	= animationsNode.GetString("Name");

		rModel->animations.emplace(animationUid, animationName);
	}

	LOG("[STATUS] Importer: Successfully loaded Model { %s } from Library! UID: %lu", rModel->GetAssetsFile(), rModel->GetUID());

	return ret;
}