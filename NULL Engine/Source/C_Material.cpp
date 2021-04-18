#include "VariableTypedefs.h"
#include "Macros.h"

#include "JSONParser.h"
#include "Color.h"

#include "Application.h"
#include "FileSystemDefinitions.h"
#include "M_ResourceManager.h"

#include "R_Material.h"
#include "R_Texture.h"
#include "R_Shader.h"

#include "GameObject.h"

#include "C_Material.h"

#include "MemoryManager.h"

#define MAX_MAPS 7

C_Material::C_Material(GameObject* owner) : Component(owner, ComponentType::MATERIAL),
rMaterial(nullptr),
rTexture(nullptr),
rShader(nullptr),
useDefaultTex(false),
useAlbedoTex(true)
{
	
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

	if (rMaterial != nullptr)
	{
		App->resourceManager->FreeResource(rMaterial->GetUID());
		rMaterial = nullptr;
	}
	if (rTexture != nullptr)
	{
		App->resourceManager->FreeResource(rTexture->GetUID());
		rTexture = nullptr;
	}

	return ret;
}

bool C_Material::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());
	
	// --- R_MATERIAL ---
	if (rMaterial != nullptr)
	{
		ParsonNode material = root.SetNode("Material");

		material.SetNumber("UID", rMaterial->GetUID());
		material.SetFloat4("Color", (math::float4) &rMaterial->diffuseColor);
		material.SetString("Name", rMaterial->GetAssetsFile());
		material.SetString("Path", rMaterial->GetLibraryPath());
		material.SetString("File", rMaterial->GetLibraryFile());
	}

	// --- R_TEXTURE ---
	if (rTexture != nullptr)
	{
		ParsonNode texture = root.SetNode("Texture");

		texture.SetString("AssetsPath", rTexture->GetAssetsPath());
		texture.SetString("AssetsName", rTexture->GetAssetsFile());
		texture.SetString("LibaryPath", rTexture->GetLibraryPath());
		texture.SetString("LibraryFile", rTexture->GetLibraryFile());
	}

	// --- R_SHADER ---
	if (rShader != nullptr)
	{
		ParsonNode shader = root.SetNode("Shader");

		shader.SetString("AssetsPath", rShader->GetAssetsPath());
		shader.SetString("AssetsName", rShader->GetAssetsFile());
		shader.SetString("LibaryPath", rShader->GetLibraryPath());
		shader.SetString("LibraryFile", rShader->GetLibraryFile());
	}
	return ret;
}

bool C_Material::LoadState(ParsonNode& root)
{
	rMaterial	= nullptr;
	rTexture	= nullptr;
	rShader		= nullptr;

	ParsonNode materialNode	= root.GetNode("Material", false);
	ParsonNode textureNode	= root.GetNode("Texture", false);
	ParsonNode shaderNode	= root.GetNode("Shader", false);

	(materialNode.NodeIsValid())	? LoadMaterial(materialNode)	: LOG("[WARNING] Loading Scene: Could not find any Material for { %s }! Check if intended.", this->GetOwner()->GetName());
	(textureNode.NodeIsValid())		? LoadTexture(textureNode)		: LOG("[WARNING] Loading Scene: Could not find any Texture for { %s }! Check if intended.", this->GetOwner()->GetName());
	(shaderNode.NodeIsValid())		? LoadShader(shaderNode)		: LOG("[WARNING] Loading Scene: Could not find any Shader for { %s }! Check if intended.", this->GetOwner()->GetName());

	return true;
}

// --- C_MATERIAL METHODS ---
void C_Material::LoadMaterial(ParsonNode& materialNode)
{
	std::string matAssetsPath	= ASSETS_MODELS_PATH + std::string(materialNode.GetString("Name"));
	bool success				= App->resourceManager->AllocateResource((uint32)materialNode.GetNumber("UID"), matAssetsPath.c_str());
	if (success)
	{
		rMaterial = (R_Material*)App->resourceManager->RequestResource((uint32)materialNode.GetNumber("UID"));
		SetMaterialColour(materialNode.GetFloat4("Color").ptr());
	}

	if (rMaterial == nullptr)
	{
		LOG("[ERROR] Loading Scene: Could not find Material { %s } with UID: %u! Try reimporting the model.", materialNode.GetString("File"), (uint32)materialNode.GetNumber("UID"));
	}
}

void C_Material::LoadTexture(const ParsonNode& textureNode)
{
	rTexture = (R_Texture*)App->resourceManager->GetResourceFromLibrary(textureNode.GetString("AssetsPath"));

	if (rTexture == nullptr)
	{
		LOG("[ERROR] Loading Scene: Could not find Texture { %s } with UID: %u! Try reimporting the model.", textureNode.GetString("LibraryFile"), (uint32)textureNode.GetNumber("UID"));
	}
}

void C_Material::LoadShader(const ParsonNode& shaderNode)
{
	rShader = (R_Shader*)App->resourceManager->GetResourceFromLibrary(shaderNode.GetString("AssetsPath"));

	if (rShader == nullptr)
	{
		LOG("[ERROR] Loading Scene: Could not find Shader { %s } with UID: %u! Try reimporting the model.", shaderNode.GetString("LibraryFile"), (uint32)shaderNode.GetNumber("UID"));
	}
}

// --- C_MATERIAL GET/SET METHODS ---
R_Material* C_Material::GetMaterial() const
{
	return rMaterial;
}

R_Texture* C_Material::GetTexture() const
{
	return rTexture;
}

R_Shader* C_Material::GetShader() const
{
	return rShader;
}

void C_Material::SetMaterial(R_Material* rMaterial)
{
	if (this->rMaterial != nullptr)
	{
		App->resourceManager->FreeResource(this->rMaterial->GetUID());
	}
	
	this->rMaterial = rMaterial;
}

void C_Material::SetTexture(R_Texture* rTexture)
{

	if (this->rTexture != nullptr)
	{
		App->resourceManager->FreeResource(this->rTexture->GetUID());
	}

	this->rTexture = rTexture;
}

void C_Material::SwapTexture(R_Texture* rTexture)
{
	this->rTexture = rTexture;
}

void C_Material::SetShader(R_Shader* rShader)
{
	this->rShader = rShader;
}

Color C_Material::GetMaterialColour()
{
	if (rMaterial != nullptr)
	{
		return rMaterial->diffuseColor;
	}

	return Color();
}

void C_Material::SetMaterialColour(const Color& color)
{
	if (rMaterial != nullptr)
	{
		rMaterial->diffuseColor = color;
	}
}

// --- MATERIAL AND TEXTURE GET/SET DATA METHODS
void C_Material::SetMaterialColour(float r, float g, float b, float a)
{
	if (rMaterial != nullptr)
	{
		rMaterial->diffuseColor = Color(r, g, b, a);
	}
	else
	{
		LOG("[ERROR] Material Component of %s has no Material Resource!", this->GetOwner()->GetName());
	}
}

void C_Material::AddTextureMap(TextureMap textureMap, uint texId)
{
	//texture_maps[(uint)textureMap] = texId;
}

void C_Material::SetCurrentTextureMap(TextureMap textureMap)
{
	// currentMap = textureMap;
	// texId = textureMaps[(uint)currentMap];
}

bool C_Material::UseDefaultTexture() const
{
	return useDefaultTex;
}

void C_Material::SetUseDefaultTexture(const bool& setTo)
{
	useDefaultTex = setTo;
}

const char* C_Material::GetTexturePath() const
{
	return (rTexture != nullptr) ? rTexture->GetAssetsPath() : "NONE";
}

const char* C_Material::GetTextureFile() const
{
	return (rTexture != nullptr) ? rTexture->GetAssetsFile() : "NONE";
}

uint C_Material::GetTextureID() const
{
	return (rTexture != nullptr) ? rTexture->GetTextureID() : 0;
}

void C_Material::GetTextureSize(uint& width, uint& height)
{
	if (rTexture != nullptr)
	{
		width = rTexture->GetTextureWidth();
		height = rTexture->GetTextureHeight();
	}
	else
	{
		LOG("[ERROR] Material Component of %s has no Texture Resource!", this->GetOwner()->GetName());
	}
}

void C_Material::GetTextureInfo(uint& id, uint& width, uint& height, uint& depth, uint& bpp, uint& bytes, std::string& format, bool& compressed)
{
	if (rTexture != nullptr)
	{
		id = rTexture->GetTextureID();
		width = rTexture->GetTextureWidth();
		height = rTexture->GetTextureHeight();
		depth = rTexture->GetTextureDepth();
		bpp = rTexture->GetTextureBpp();
		bytes = rTexture->GetTextureBytes();
		format = rTexture->GetTextureFormatString();
		compressed = rTexture->TextureIsCompressed();
	}
	else
	{
		//LOG("[ERROR] Material Component of %s has no Texture Resource!", owner->GetName());
	}
}

uint32 const C_Material::GetShaderProgramID()
{
	return rShader->shaderProgramID;
}

void C_Material::SetShaderProgramID(uint32 ID)
{
	rShader->shaderProgramID = ID;
}
