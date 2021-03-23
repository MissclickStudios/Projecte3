#include "FileSystemDefinitions.h"
#include "VariableTypedefs.h"

#include "JSONParser.h"

#include "R_Model.h"

#include "MemoryManager.h"

R_Model::R_Model() : Resource(ResourceType::MODEL)
{

}

R_Model::~R_Model()
{

}

bool R_Model::CleanUp()
{
	bool ret = true;

	modelNodes.clear();
	animations.clear();

	return ret;
}

bool R_Model::SaveMeta(ParsonNode& metaRoot) const
{
	bool ret = true;

	ParsonArray contained_array = metaRoot.SetArray("ContainedResources");

	for (uint i = 0; i < modelNodes.size(); ++i)
	{	
		if (modelNodes[i].meshUID != 0)
		{
			std::string mesh_name = modelNodes[i].name + MESHES_EXTENSION;
			std::string mesh_path = MESHES_PATH + std::to_string(modelNodes[i].meshUID) + MESHES_EXTENSION;

			ParsonNode mesh_node = contained_array.SetNode(mesh_name.c_str());

			mesh_node.SetNumber("UID", modelNodes[i].meshUID);
			mesh_node.SetNumber("Type", (uint)ResourceType::MESH);
			mesh_node.SetString("Name", mesh_name.c_str());
			mesh_node.SetString("LibraryPath", mesh_path.c_str());
		}

		if (modelNodes[i].materialUID != 0)
		{
			std::string material_name = modelNodes[i].name + MATERIALS_EXTENSION;
			std::string material_path = MATERIALS_PATH + std::to_string(modelNodes[i].materialUID) + MATERIALS_EXTENSION;

			ParsonNode material_node = contained_array.SetNode(material_name.c_str());

			material_node.SetNumber("UID", modelNodes[i].materialUID);
			material_node.SetNumber("Type", (uint)ResourceType::MATERIAL);
			material_node.SetString("Name", material_name.c_str());
			material_node.SetString("LibraryPath", material_path.c_str());
		}

		if (modelNodes[i].textureUID != 0)
		{
			std::string texture_name = modelNodes[i].textureName;
			std::string texture_path = TEXTURES_PATH + std::to_string(modelNodes[i].textureUID) + TEXTURES_EXTENSION;

			ParsonNode texture_node = contained_array.SetNode(texture_name.c_str());

			texture_node.SetNumber("UID", modelNodes[i].textureUID);
			texture_node.SetNumber("Type", (uint)ResourceType::TEXTURE);
			texture_node.SetString("Name", texture_name.c_str());
			texture_node.SetString("LibraryPath", texture_path.c_str());
		}
		if (modelNodes[i].shaderUID != 0)
		{
			std::string shader_name = modelNodes[i].name + SHADERS_EXTENSION;
			std::string shader_path = SHADERS_PATH + std::to_string(modelNodes[i].shaderUID) + SHADERS_EXTENSION;

			ParsonNode texture_node = contained_array.SetNode(shader_name.c_str());

			texture_node.SetNumber("UID", modelNodes[i].shaderUID);
			texture_node.SetNumber("Type", (uint)ResourceType::SHADER);
			texture_node.SetString("Name", shader_name.c_str());
			texture_node.SetString("LibraryPath", shader_path.c_str());
		}
	}

	std::map<uint32, std::string>::const_iterator item;
	for (item = animations.cbegin(); item != animations.cend(); ++item)
	{
		std::string animation_name = item->second + ANIMATIONS_EXTENSION;
		std::string animation_path = ANIMATIONS_PATH + std::to_string(item->first) + ANIMATIONS_EXTENSION;
		
		ParsonNode animation_node = contained_array.SetNode(animation_name.c_str());

		animation_node.SetNumber("UID", item->first);
		animation_node.SetNumber("Type", (uint)ResourceType::ANIMATION);
		animation_node.SetString("Name", animation_name.c_str());
		animation_node.SetString("LibraryPath", animation_path.c_str());
	}

	ParsonNode settings = metaRoot.SetNode("ImportSettings");
	modelSettings.Save(settings);

	return ret;
}

bool R_Model::LoadMeta(const ParsonNode& metaRoot)
{
	bool ret = true;



	return ret;
}

// --- MODEL NODE METHODS ---
ModelNode::ModelNode() :
name			("[NONE]"),
uid				(0),
parentUID		(0),
transform		(Transform()),
meshUID			(0),
materialUID		(0),
textureUID		(0),
shaderUID		(0),
textureName		("[NONE]")
{

}

ModelNode::ModelNode(const char* name, uint32 UID, uint32 parentUID, Transform transform, uint32 meshUID, uint32 materialUID, uint32 textureUID,uint32 shaderUID, std::string textureName) :
name			(name),
uid				(UID),
parentUID		(parentUID),
transform		(transform),
meshUID			(meshUID),
materialUID		(materialUID),
textureUID		(textureUID),
shaderUID		(shaderUID),
textureName		(textureName)
{

}

bool ModelNode::Save(ParsonNode& root) const
{
	bool ret = true;

	root.SetString("Name", name.c_str());

	root.SetNumber("UID", uid);
	root.SetNumber("ParentUID", parentUID);

	ParsonNode transformNode	= root.SetNode("Transform");
	ParsonArray position		= transformNode.SetArray("LocalPosition");
	ParsonArray rotation		= transformNode.SetArray("LocalRotation");
	ParsonArray scale			= transformNode.SetArray("LocalScale");

	position.SetFloat3(transform.position);
	rotation.SetFloat4(transform.rotation.CastToFloat4());
	scale.SetFloat3(transform.scale);

	root.SetNumber("MeshUID", meshUID);
	root.SetNumber("MaterialUID", materialUID);
	root.SetNumber("TextureUID", textureUID);
	root.SetNumber("ShaderUID", shaderUID);

	root.SetString("TextureName", textureName.c_str());

	return ret;
}

bool ModelNode::Load(const ParsonNode& root)
{
	bool ret = true;

	name = root.GetString("Name");

	uid				= (uint32)root.GetNumber("UID");
	parentUID		= (uint32)root.GetNumber("ParentUID");

	ParsonNode transformNode	= root.GetNode("Transform");
	ParsonArray position		= transformNode.GetArray("LocalPosition");
	ParsonArray rotation		= transformNode.GetArray("LocalRotation");
	ParsonArray scale			= transformNode.GetArray("LocalScale");

	position.GetFloat3(0, transform.position);
	rotation.GetFloat4(0, transform.rotation);
	scale.GetFloat3(0, transform.scale);

	meshUID			= (uint32)root.GetNumber("MeshUID");
	materialUID		= (uint32)root.GetNumber("MaterialUID");
	textureUID		= (uint32)root.GetNumber("TextureUID");
	shaderUID		= (uint32)root.GetNumber("ShaderUID");

	textureName		= root.GetString("TextureName");

	return ret;
}
