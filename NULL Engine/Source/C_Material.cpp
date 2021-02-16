#include "VariableTypedefs.h"
#include "Macros.h"

#include "JSONParser.h"
#include "Color.h"

#include "Application.h"
#include "M_ResourceManager.h"

#include "R_Material.h"
#include "R_Texture.h"

#include "GameObject.h"

#include "C_Material.h"

#define MAX_MAPS 7

C_Material::C_Material(GameObject* owner) : Component(owner, COMPONENT_TYPE::MATERIAL),
r_material			(nullptr),
r_texture			(nullptr),
use_default_tex		(false),
use_albedo_tex		(true)
{
	//memset(texture_maps, 0, sizeof(uint) * MAX_MAPS);
}

C_Material::~C_Material()
{

}

bool C_Material::Update()
{
	bool ret = true;

	return ret;
}

bool C_Material::CleanUp()
{
	bool ret = true;

	if (r_material != nullptr)
	{
		App->resource_manager->FreeResource(r_material->GetUID());
		r_material = nullptr;
	}
	if (r_texture != nullptr)
	{
		App->resource_manager->FreeResource(r_texture->GetUID());
		r_texture = nullptr;
	}

	return ret;
}

bool C_Material::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());
	
	// --- R_MATERIAL ---
	if (r_material != nullptr)
	{
		ParsonNode material = root.SetNode("Material");

		material.SetNumber("UID", r_material->GetUID());
		material.SetString("Name", r_material->GetAssetsFile());
		material.SetString("Path", r_material->GetLibraryPath());
		material.SetString("File", r_material->GetLibraryFile());
	}

	// --- R_TEXTURE ---
	if (r_texture != nullptr)
	{
		ParsonNode texture = root.SetNode("Texture");

		texture.SetNumber("UID", r_texture->GetUID());
		texture.SetString("Name", r_texture->GetAssetsFile());
		texture.SetString("Path", r_texture->GetLibraryPath());
		texture.SetString("File", r_texture->GetLibraryFile());
	}

	return ret;
}

bool C_Material::LoadState(ParsonNode& root)
{
	bool ret = true;

	r_material	= nullptr;
	r_texture	= nullptr;

	ParsonNode material_node	= root.GetNode("Material");
	ParsonNode texture_node		= root.GetNode("Texture");
	
	if (material_node.NodeIsValid())
	{
		std::string material_assets_path = ASSETS_MODELS_PATH + std::string(material_node.GetString("Name"));
		App->resource_manager->AllocateResource((uint32)material_node.GetNumber("UID"), material_assets_path.c_str());

		r_material = (R_Material*)App->resource_manager->RequestResource((uint32)material_node.GetNumber("UID"));

		if (r_material == nullptr)
		{
			LOG("[ERROR] Loading Scene: Could not find Material %s with UID: %u! Try reimporting the model.", material_node.GetString("File"), (uint32)material_node.GetNumber("UID"));
		}
	}

	if (texture_node.NodeIsValid())
	{
		std::string texture_assets_path	= ASSETS_TEXTURES_PATH + std::string(texture_node.GetString("Name"));
		App->resource_manager->AllocateResource((uint32)texture_node.GetNumber("UID"), texture_assets_path.c_str());

		r_texture = (R_Texture*)App->resource_manager->RequestResource((uint32)texture_node.GetNumber("UID"));

		if (r_texture == nullptr)
		{
			LOG("[ERROR] Loading Scene: Could not find Texture %s with UID: %u! Try reimporting the model.", texture_node.GetString("File"), (uint32)texture_node.GetNumber("UID"));
		}
	}
	else
	{
		LOG("[WARNING] Loading Scene: Could not find any Texture for %s! Check whether or not this is intended.", this->GetOwner()->GetName());
	}

	return ret;
}

// --- C_MATERIAL METHODS ---
R_Material* C_Material::GetMaterial() const
{
	return r_material;
}

R_Texture* C_Material::GetTexture() const
{
	return r_texture;
}

void C_Material::SetMaterial(R_Material* r_material)
{
	if (this->r_material != nullptr)
	{
		App->resource_manager->FreeResource(this->r_material->GetUID());
	}
	
	this->r_material = r_material;
}

void C_Material::SetTexture(R_Texture* r_texture)
{

	if (this->r_texture != nullptr)
	{
		App->resource_manager->FreeResource(this->r_texture->GetUID());
	}

	this->r_texture = r_texture;
}

Color C_Material::GetMaterialColour()
{
	if (r_material != nullptr)
	{
		return r_material->diffuse_color;
	}

	return Color();
}

void C_Material::SetMaterialColour(const Color& color)
{
	r_material->diffuse_color = color;
}

// --- MATERIAL AND TEXTURE GET/SET DATA METHODS
void C_Material::SetMaterialColour(float r, float g, float b, float a)
{
	if (r_material != nullptr)
	{
		r_material->diffuse_color = Color(r, g, b, a);
	}
	else
	{
		LOG("[ERROR] Material Component of %s has no Material Resource!", this->GetOwner()->GetName());
	}
}

void C_Material::AddTextureMap(TEXTURE_MAP texture_map, uint tex_id)
{
	//texture_maps[(uint)texture_map] = tex_id;
}

void C_Material::SetCurrentTextureMap(TEXTURE_MAP texture_map)
{
	// current_map = texture_map;
	// tex_id		= texture_maps[(uint)current_map];
}

bool C_Material::UseDefaultTexture() const
{
	return use_default_tex;
}

void C_Material::SetUseDefaultTexture(const bool& set_to)
{
	use_default_tex = set_to;
}

const char* C_Material::GetTexturePath() const
{
	return (r_texture != nullptr) ? r_texture->GetAssetsPath() : "NONE";
}

const char* C_Material::GetTextureFile() const
{
	return (r_texture != nullptr) ? r_texture->GetAssetsFile() : "NONE";
}

uint C_Material::GetTextureID() const
{
	return (r_texture != nullptr) ? r_texture->GetTextureID() : 0;
}

void C_Material::GetTextureSize(uint& width, uint& height)
{
	if (r_texture != nullptr)
	{
		width	= r_texture->GetTextureWidth();
		height	= r_texture->GetTextureHeight();
	}
	else
	{
		LOG("[ERROR] Material Component of %s has no Texture Resource!", this->GetOwner()->GetName());
	}
}

void C_Material::GetTextureInfo(uint& id, uint& width, uint& height, uint& depth, uint& bpp, uint& bytes, std::string& format, bool& compressed)
{
	if (r_texture != nullptr)
	{
		id			= r_texture->GetTextureID();
		width		= r_texture->GetTextureWidth();
		height		= r_texture->GetTextureHeight();
		depth		= r_texture->GetTextureDepth();
		bpp			= r_texture->GetTextureBpp();
		bytes		= r_texture->GetTextureBytes();
		format		= r_texture->GetTextureFormatString();
		compressed	= r_texture->TextureIsCompressed();
	}
	else
	{
		//LOG("[ERROR] Material Component of %s has no Texture Resource!", owner->GetName());
	}
}