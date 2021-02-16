#include "FileSystemDefinitions.h"
#include "VariableTypedefs.h"

#include "JSONParser.h"

#include "R_Model.h"

R_Model::R_Model() : Resource(RESOURCE_TYPE::MODEL)
{

}

R_Model::~R_Model()
{

}

bool R_Model::CleanUp()
{
	bool ret = true;

	model_nodes.clear();
	animations.clear();

	return ret;
}

bool R_Model::SaveMeta(ParsonNode& meta_root) const
{
	bool ret = true;

	ParsonArray contained_array = meta_root.SetArray("ContainedResources");

	for (uint i = 0; i < model_nodes.size(); ++i)
	{	
		if (model_nodes[i].mesh_uid != 0)
		{
			std::string mesh_name = model_nodes[i].name + MESHES_EXTENSION;
			std::string mesh_path = MESHES_PATH + std::to_string(model_nodes[i].mesh_uid) + MESHES_EXTENSION;

			ParsonNode mesh_node = contained_array.SetNode(mesh_name.c_str());

			mesh_node.SetNumber("UID", model_nodes[i].mesh_uid);
			mesh_node.SetNumber("Type", (uint)RESOURCE_TYPE::MESH);
			mesh_node.SetString("Name", mesh_name.c_str());
			mesh_node.SetString("LibraryPath", mesh_path.c_str());
		}

		if (model_nodes[i].material_uid != 0)
		{
			std::string material_name = model_nodes[i].name + MATERIALS_EXTENSION;
			std::string material_path = MATERIALS_PATH + std::to_string(model_nodes[i].material_uid) + MATERIALS_EXTENSION;

			ParsonNode material_node = contained_array.SetNode(material_name.c_str());

			material_node.SetNumber("UID", model_nodes[i].material_uid);
			material_node.SetNumber("Type", (uint)RESOURCE_TYPE::MATERIAL);
			material_node.SetString("Name", material_name.c_str());
			material_node.SetString("LibraryPath", material_path.c_str());
		}

		if (model_nodes[i].texture_uid != 0)
		{
			std::string texture_name = model_nodes[i].texture_name;
			std::string texture_path = TEXTURES_PATH + std::to_string(model_nodes[i].texture_uid) + TEXTURES_EXTENSION;

			ParsonNode texture_node = contained_array.SetNode(texture_name.c_str());

			texture_node.SetNumber("UID", model_nodes[i].texture_uid);
			texture_node.SetNumber("Type", (uint)RESOURCE_TYPE::TEXTURE);
			texture_node.SetString("Name", texture_name.c_str());
			texture_node.SetString("LibraryPath", texture_path.c_str());
		}
	}

	std::map<uint32, std::string>::const_iterator item;
	for (item = animations.cbegin(); item != animations.cend(); ++item)
	{
		std::string animation_name = item->second;
		std::string animation_path = ANIMATIONS_PATH + std::to_string(item->first) + ANIMATIONS_EXTENSION;
		
		ParsonNode animation_node = contained_array.SetNode(animation_name.c_str());

		animation_node.SetNumber("UID", item->first);
		animation_node.SetNumber("Type", (uint)RESOURCE_TYPE::ANIMATION);
		animation_node.SetString("Name", animation_name.c_str());
		animation_node.SetString("LibraryPath", animation_path.c_str());
	}

	ParsonNode settings = meta_root.SetNode("ImportSettings");
	model_settings.Save(settings);

	return ret;
}

bool R_Model::LoadMeta(const ParsonNode& meta_root)
{
	bool ret = true;



	return ret;
}

// --- MODEL NODE METHODS ---
ModelNode::ModelNode() :
name			("[NONE]"),
uid				(0),
parent_uid		(0),
transform		(Transform()),
mesh_uid		(0),
material_uid	(0),
texture_uid		(0),
texture_name	("[NONE]")
{

}

ModelNode::ModelNode(const char* name, uint32 UID, uint32 parent_UID, Transform transform, uint32 mesh_UID, uint32 material_UID, uint32 texture_UID, std::string texture_name) :
name			(name),
uid				(UID),
parent_uid		(parent_UID),
transform		(transform),
mesh_uid		(mesh_UID),
material_uid	(material_UID),
texture_uid		(texture_UID),
texture_name	(texture_name)
{

}

bool ModelNode::Save(ParsonNode& root) const
{
	bool ret = true;

	root.SetString("Name", name.c_str());

	root.SetNumber("UID", uid);
	root.SetNumber("ParentUID", parent_uid);

	ParsonNode transform_node	= root.SetNode("Transform");
	ParsonArray position		= transform_node.SetArray("LocalPosition");
	ParsonArray rotation		= transform_node.SetArray("LocalRotation");
	ParsonArray scale			= transform_node.SetArray("LocalScale");

	position.SetFloat3(transform.position);
	rotation.SetFloat4(transform.rotation.CastToFloat4());
	scale.SetFloat3(transform.scale);

	root.SetNumber("MeshUID", mesh_uid);
	root.SetNumber("MaterialUID", material_uid);
	root.SetNumber("TextureUID", texture_uid);

	root.SetString("TextureName", texture_name.c_str());

	return ret;
}

bool ModelNode::Load(const ParsonNode& root)
{
	bool ret = true;

	name = root.GetString("Name");

	uid				= (uint32)root.GetNumber("UID");
	parent_uid		= (uint32)root.GetNumber("ParentUID");

	ParsonNode transform_node	= root.GetNode("Transform");
	ParsonArray position		= transform_node.GetArray("LocalPosition");
	ParsonArray rotation		= transform_node.GetArray("LocalRotation");
	ParsonArray scale			= transform_node.GetArray("LocalScale");

	position.GetFloat3(0, transform.position);
	rotation.GetFloat4(0, transform.rotation);
	scale.GetFloat3(0, transform.scale);

	mesh_uid		= (uint32)root.GetNumber("MeshUID");
	material_uid	= (uint32)root.GetNumber("MaterialUID");
	texture_uid		= (uint32)root.GetNumber("TextureUID");

	texture_name	= root.GetString("TextureName");

	return ret;
}
