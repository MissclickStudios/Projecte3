#include "VariableTypedefs.h"

#include "JSONParser.h"

#include "R_Material.h"

R_Material::R_Material() : Resource(RESOURCE_TYPE::MATERIAL)
{
	diffuse_color	= { 1.0f, 1.0f, 1.0f, 1.0f };
}

R_Material::~R_Material()
{

}

bool R_Material::CleanUp()
{
	bool ret = true;

	for (uint i = 0; i < materials.size(); ++i)
	{
		materials[i].CleanUp();
	}

	materials.clear();

	return ret;
}

bool R_Material::SaveMeta(ParsonNode& meta_root) const
{
	bool ret = true;

	ParsonArray contained_array = meta_root.SetArray("ContainedResources");

	ParsonNode settings = meta_root.SetNode("ImportSettings");
	material_settings.Save(settings);

	return ret;
}

bool R_Material::LoadMeta(const ParsonNode& meta_root)
{
	bool ret = true;



	return ret;
}

// --- MATERIAL METHODS ---
MaterialData::MaterialData() :
type				(TEXTURE_TYPE::NONE),
texture_uid			(0),
texture_assets_path	("[NONE]")
{

}

MaterialData::MaterialData(TEXTURE_TYPE type, uint texture_uid, std::string texture_assets_path) :
type				(type),
texture_uid			(texture_uid),
texture_assets_path	(texture_assets_path)
{

}

void MaterialData::CleanUp()
{
	texture_assets_path.clear();
	texture_assets_path.shrink_to_fit();
}