#include "VariableTypedefs.h"

#include "JSONParser.h"

#include "R_Material.h"

#include "MemoryManager.h"

R_Material::R_Material() : Resource(ResourceType::MATERIAL)
{
	diffuseColor = Color(1.0f, 1.0f, 1.0f, 1.0f );
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

bool R_Material::SaveMeta(ParsonNode& metaRoot) const
{
	bool ret = true;

	ParsonArray contained_array = metaRoot.SetArray("ContainedResources");

	ParsonNode settings = metaRoot.SetNode("ImportSettings");
	materialSettings.Save(settings);

	return ret;
}

bool R_Material::LoadMeta(const ParsonNode& metaRoot)
{
	bool ret = true;



	return ret;
}

void R_Material::SetColor(Color color)
{
	diffuseColor = color;
}

// --- MATERIAL METHODS ---
MaterialData::MaterialData() :
type				(TextureType::NONE),
textureUID			(0),
textureAssetsPath	("[NONE]")
{

}

MaterialData::MaterialData(TextureType type, uint textureUID, std::string textureAssetsPath) :
type				(type),
textureUID			(textureUID),
textureAssetsPath	(textureAssetsPath)
{

}

void MaterialData::CleanUp()
{
	textureAssetsPath.clear();
	textureAssetsPath.shrink_to_fit();
}