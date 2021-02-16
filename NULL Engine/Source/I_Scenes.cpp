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

void Importer::Scenes::Import(const char* buffer, uint size, R_Model* r_model)
{
	if (r_model == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Model! Error: R_Model* was nullptr.");
		return;
	}
	
	std::string error_string = "[ERROR] Importer: Could not Import Model { " + std::string(r_model->GetAssetsFile()) + " }";
	
	if (buffer == nullptr)
	{
		LOG("%s! Error: Buffer was nullptr.", error_string.c_str());
		return;
	}
	if (size == 0)
	{
		LOG("%s! Error: Size was 0.", error_string.c_str());
		return;
	}
	
	LOG("[STATUS] Importing Scene: %s", r_model->GetAssetsFile());

	const aiScene* ai_scene = aiImportFileFromMemory(buffer, size, aiProcessPreset_TargetRealtime_MaxQuality, nullptr);

	if (ai_scene == nullptr || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode)
	{
		LOG("%s! Error: Assimp Error [%s]", error_string.c_str(), aiGetErrorString());
		return;
	}

	for (uint i = 0; i < ai_scene->mNumMeshes; ++i)
	{
		Utilities::ai_meshes.push_back(ai_scene->mMeshes[i]);
	
		uint mat_index = ai_scene->mMeshes[i]->mMaterialIndex;
		if (mat_index >= 0)
		{
			Utilities::ai_materials.push_back(ai_scene->mMaterials[mat_index]);
		}
	}

	Utilities::ProcessNode(ai_scene, ai_scene->mRootNode, r_model, ModelNode());							// First Parent is empty. Later assigned to scene_root.

	Utilities::ImportAnimations(ai_scene, r_model);

	Utilities::ai_meshes.clear();
	Utilities::ai_materials.clear();
	Utilities::loaded_nodes.clear();
	Utilities::loaded_textures.clear();
}

void Importer::Scenes::Utilities::ProcessNode(const aiScene* ai_scene, const aiNode* ai_node, R_Model* r_model, const ModelNode& parent)
{
	ModelNode model_node	= ModelNode();
	model_node.uid			= Random::LCG::GetRandomUint();
	model_node.parent_uid	= parent.uid;

	ai_node = Utilities::ImportTransform(ai_node, model_node);
	Utilities::ImportMeshesAndMaterials(ai_scene, ai_node, r_model, model_node);

	model_node.name	= (ai_node == ai_scene->mRootNode) ? r_model->GetAssetsFile() : ai_node->mName.C_Str();

	r_model->model_nodes.push_back(model_node);

	for (uint i = 0; i < ai_node->mNumChildren; ++i)
	{
		ProcessNode(ai_scene, ai_node->mChildren[i], r_model, model_node);
	}
}

const aiNode* Importer::Scenes::Utilities::ImportTransform(const aiNode* ai_node, ModelNode& model_node)
{
	// Assimp generates dummy nodes to store multiple FBX transformations.
	// All those transformations will be collapsed to the first non-dummy node.
	
	aiTransform ai_t;																						// Transform structure for Assimp. aiVector3D and aiQuaternion.
	Transform	ma_t;																						// Transform structure for MathGeoLib. float3 and Quat.

	ai_node->mTransformation.Decompose(ai_t.scale, ai_t.rotation, ai_t.position);							// --- Getting the Transform stored in the node.

	ma_t.position	= { ai_t.position.x, ai_t.position.y, ai_t.position.z };								// 
	ma_t.rotation	= { ai_t.rotation.x, ai_t.rotation.y, ai_t.rotation.z, ai_t.rotation.w };				// 
	ma_t.scale		= { ai_t.scale.x, ai_t.scale.y, ai_t.scale.z };											// ---------------------------------------------

	while (NodeIsDummyNode(*ai_node))																		// All dummy nodes will contain the "_$AssimpFbx$_" string and only one child node.
	{
		ai_node = ai_node->mChildren[0];																	// As dummies will only have one child, selecting the next one to process is easy.

		ai_node->mTransformation.Decompose(ai_t.scale, ai_t.rotation, ai_t.position);						// --- Getting the Transform stored in the dummy node.

		Transform dummy;																					// 
		dummy.position	= { ai_t.position.x, ai_t.position.y, ai_t.position.z };							// 
		dummy.rotation	= { ai_t.rotation.x, ai_t.rotation.y, ai_t.rotation.z, ai_t.rotation.w };			// 
		dummy.scale		= { ai_t.scale.x, ai_t.scale.y, ai_t.scale.z };										// ---------------------------------------------------

		/*ma_t.position	= */ma_t.position.Add(dummy.position);												// --- Adding the dummy's Transform to the current one.
		/*ma_t.rotation	= */ma_t.rotation.Mul(dummy.rotation);												// 
		/*ma_t.scale	= */ma_t.scale.Mul(dummy.scale);													// ----------------------------------------------------
	}
	
	model_node.transform	= ma_t;

	LOG("[IMPORTER] Imported the transforms of node: %s", ai_node->mName.C_Str());

	return ai_node;
}

void Importer::Scenes::Utilities::ImportMeshesAndMaterials(const aiScene* ai_scene, const aiNode* ai_node, R_Model* r_model, ModelNode& model_node)
{
	if (ai_scene == nullptr || ai_node == nullptr || r_model == nullptr)
	{
		return;
	}
	if (!ai_scene->HasMeshes())
	{
		return;
	}
	
	const char* node_name = ai_node->mName.C_Str();

	for (uint i = 0; i < ai_node->mNumMeshes; ++i)
	{	
		std::map<uint, ModelNode>::iterator item = loaded_nodes.find(ai_node->mMeshes[i]);
		if (item != loaded_nodes.end())
		{
			model_node.mesh_uid		= item->second.mesh_uid;
			model_node.material_uid = item->second.material_uid;
			model_node.texture_uid	= item->second.texture_uid;
			continue;
		}
		
		aiMesh* ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];

		if (ai_mesh != nullptr && ai_mesh->HasFaces())
		{
			Importer::Scenes::Utilities::ImportMesh(node_name, ai_mesh, model_node);

			if (ai_mesh->mMaterialIndex >= 0)
			{
				aiMaterial* ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

				Importer::Scenes::Utilities::ImportMaterial(node_name, ai_material, r_model, model_node);
			}
		}

		loaded_nodes.emplace(ai_node->mMeshes[i], model_node);
	}
}

void Importer::Scenes::Utilities::ImportMesh(const char* node_name, const aiMesh* ai_mesh, ModelNode& model_node)
{
	std::string assets_path = ASSETS_MODELS_PATH + std::string(node_name) + MESHES_EXTENSION;						// As meshes are contained in models, the assets path is kind of made-up.
	R_Mesh* r_mesh = (R_Mesh*)App->resource_manager->CreateResource(RESOURCE_TYPE::MESH, assets_path.c_str());

	Importer::Meshes::Import(ai_mesh, r_mesh);

	if (r_mesh == nullptr)
	{
		return;
	}
	
	model_node.mesh_uid = r_mesh->GetUID();
	App->resource_manager->SaveResourceToLibrary(r_mesh);
	App->resource_manager->DeallocateResource(r_mesh);
}

void Importer::Scenes::Utilities::ImportMaterial(const char* node_name, const aiMaterial* ai_material, R_Model* r_model, ModelNode& model_node)
{
	std::string mat_full_path	= App->file_system->GetDirectory(r_model->GetAssetsPath()) + node_name + MATERIALS_EXTENSION;
	R_Material* r_material		= (R_Material*)App->resource_manager->CreateResource(RESOURCE_TYPE::MATERIAL, mat_full_path.c_str());			// Only considering one texture per mesh.

	if (r_material == nullptr)
	{
		return;
	}
	
	Importer::Materials::Import(ai_material, r_material);
	
	model_node.material_uid = r_material->GetUID();																								//

	Utilities::ImportTexture(r_material->materials, model_node);
	
	App->resource_manager->SaveResourceToLibrary(r_material);
	App->resource_manager->DeallocateResource(r_material);
}

void Importer::Scenes::Utilities::ImportTexture(const std::vector<MaterialData>& materials, ModelNode& model_node)
{
	for (uint i = 0; i < materials.size(); ++i)
	{
		const char* tex_path	= materials[i].texture_assets_path.c_str();
		char* buffer			= nullptr;
		uint read				= App->file_system->Load(tex_path, &buffer);
		if (buffer != nullptr && read > 0)
		{
			std::map<std::string, uint32>::iterator item = loaded_textures.find(tex_path);
			if (item != loaded_textures.end())
			{
				if (materials[i].type == TEXTURE_TYPE::DIFFUSE)
				{
					model_node.texture_uid = item->second;
					model_node.texture_name = App->file_system->GetFileAndExtension(tex_path);
				}

				RELEASE_ARRAY(buffer);
				continue;
			}
			
			R_Texture* r_texture = (R_Texture*)App->resource_manager->CreateResource(RESOURCE_TYPE::TEXTURE, tex_path);
			uint tex_id = Importer::Textures::Import(buffer, read, r_texture);											//

			if (tex_id == 0)
			{
				App->resource_manager->DeleteResource(r_texture);
				RELEASE_ARRAY(buffer);
				continue;
			}

			if (materials[i].type == TEXTURE_TYPE::DIFFUSE)																// For now only the diffuse texture will be used on models' meshes.
			{
				model_node.texture_uid	= r_texture->GetUID();
				model_node.texture_name = r_texture->GetAssetsFile();
			}

			loaded_textures.emplace(tex_path, r_texture->GetUID());

			App->resource_manager->SaveResourceToLibrary(r_texture);
			App->resource_manager->DeallocateResource(r_texture);

			RELEASE_ARRAY(buffer);
		}
		else
		{
			LOG("[ERROR] Importer: Could not load texture from given path! Path: %s", tex_path);
		}
	}
}

void Importer::Scenes::Utilities::ImportAnimations(const aiScene* ai_scene, R_Model* r_model)
{
	if (ai_scene == nullptr)
	{
		LOG("[ERROR] Scene Importer: Could not Import Model's Animations! Error: Given aiScene* was nullptr");
		return;
	}
	if (!ai_scene->HasAnimations())
	{
		LOG("[WARNING] Scene Importer: Model had no animations to import.");
		return;
	}

	for (uint i = 0; i < ai_scene->mNumAnimations; ++i)
	{
		aiAnimation* ai_animation = ai_scene->mAnimations[i];

		std::string name			= ai_animation->mName.C_Str();
		std::string assets_path		= ASSETS_MODELS_PATH + name + ANIMATIONS_EXTENSION;
		R_Animation* r_animation	= (R_Animation*)App->resource_manager->CreateResource(RESOURCE_TYPE::ANIMATION, assets_path.c_str());

		if (r_animation == nullptr)
		{
			continue;
		}

		Importer::Animations::Import(ai_animation, r_animation);

		r_model->animations.emplace(r_animation->GetUID(), r_animation->GetName());

		App->resource_manager->SaveResourceToLibrary(r_animation);
		App->resource_manager->DeallocateResource(r_animation);
	}
}

bool Importer::Scenes::Utilities::NodeIsDummyNode(const aiNode& ai_node)
{
	return (strstr(ai_node.mName.C_Str(), "_$AssimpFbx$_") != nullptr && ai_node.mNumChildren == 1);	// All dummy nodes will contain the "_$AssimpFbx$_" string and only one child node.
}

uint Importer::Scenes::Save(const R_Model* r_model, char** buffer)
{	
	uint written = 0;

	if (r_model == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save R_Model* in Library! Error: Given R_Model* was nullptr.");
		return 0;
	}

	std::string error_string = "[ERROR] Importer: Could not Save Model { " + std::string(r_model->GetAssetsFile()) + " } from Library";

	ParsonNode root_node			= ParsonNode();																						// --- GENERATING THE REQUIRED PARSON NODE AND ARRAY
	ParsonArray model_nodes_array	= root_node.SetArray("ModelNodes");																	// -------------------------------------------------
	ParsonArray animations_array	= root_node.SetArray("Animations");

	for (uint i = 0; i < r_model->model_nodes.size(); ++i)																				// --- SAVING MODEL NODE DATA
	{
		ParsonNode model_node = model_nodes_array.SetNode(r_model->model_nodes[i].name.c_str());
		r_model->model_nodes[i].Save(model_node);
	}

	std::map<uint32, std::string>::const_iterator item;
	for (item = r_model->animations.cbegin(); item != r_model->animations.cend(); ++item)
	{
		ParsonNode animation_node = animations_array.SetNode(item->second.c_str());
		animation_node.SetNumber("UID", (double)item->first);
		animation_node.SetString("Name", item->second.c_str());
	}

	std::string path	= MODELS_PATH + std::to_string(r_model->GetUID()) + MODELS_EXTENSION;
	written				= root_node.SerializeToFile(path.c_str(), buffer);
	if (written > 0)
	{
		LOG("[STATUS] Importer: Successfully saved Model { %s } in Library! Path: %s", r_model->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("%s! Error: File System could not write the file.", error_string.c_str());
	}

	return written;
}

bool Importer::Scenes::Load(const char* buffer, R_Model* r_model)
{
	bool ret = true;

	if (r_model == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Model from Library! Error: R_Model was nullptr.");
		return false;
	}
	
	std::string error_string = "[ERROR] Importer: Could not Load Model { " + std::string(r_model->GetAssetsFile()) + " } from Library";

	if (buffer == nullptr)
	{
		LOG("%s! Error: Given buffer was nullptr.", error_string.c_str());
		return false;
	}

	ParsonNode root_node			= ParsonNode(buffer);
	ParsonArray model_nodes_array	= root_node.GetArray("ModelNodes");
	ParsonArray animations_array	= root_node.GetArray("Animations");
	if (!root_node.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Root Node from the passed buffer.", error_string.c_str());
		return false;
	}
	if (!model_nodes_array.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the ModelNodes Array from the Root Node.", error_string.c_str());
		return false;
	}
	if (!animations_array.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the Animations Array from the Root Node.", error_string.c_str());
	}

	for (uint i = 0; i < model_nodes_array.size; ++i)
	{
		ParsonNode parson_node = model_nodes_array.GetNode(i);
		if (!parson_node.NodeIsValid())
		{
			LOG("%s! Error: Could not parse Node %s from Model Nodes Array.", error_string.c_str(), i);
			return false;
		}

		ModelNode model_node = ModelNode();
		model_node.Load(parson_node);

		r_model->model_nodes.push_back(model_node);
	}

	for (uint i = 0; i < animations_array.size; ++i)
	{
		ParsonNode animations_node = animations_array.GetNode(i);
		if (!animations_node.NodeIsValid())
		{
			LOG("%s! Error: Could not parse Node %s from Animations Array.", error_string.c_str());
			return false;
		}

		uint32 animation_uid		= (uint32)animations_node.GetNumber("UID");
		std::string animation_name	= animations_node.GetString("Name");

		r_model->animations.emplace(animation_uid, animation_name);
	}

	LOG("[STATUS] Importer: Successfully loaded Model { %s } from Library! UID: %lu", r_model->GetAssetsFile(), r_model->GetUID());

	return ret;
}