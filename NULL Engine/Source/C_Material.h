#ifndef __C_MATERIAL_H__
#define __C_MATERIAL_H__

#include <vector>

#include "Color.h"

#include "Component.h"

class ParsonNode;
class R_Material;
class R_Texture;
class R_Shader;
class GameObject;

struct Texture;

typedef unsigned int uint;

enum class TextureMap												// Will correspond with the indexes of the texture_ids vector. Ex: texture_ids[DIFFUSE] = diffuse tex_id;
{
	DEFAULT = 0,												// Will correspond with the Debug Texture (Checkers).
	DIFFUSE = 1,
	SPECULAR = 2,
	AMBIENT = 3,
	EMISSIVE = 4,
	HEIGHT = 5,
	NORMAL = 6,
};

class NULL_API C_Material : public Component
{
public:
	C_Material(GameObject* owner);
	~C_Material();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::MATERIAL; }				// This is needed to be able to use templeates for functions such as GetComponent<>();

public:																						// --- GET/SET RESOURCES
	R_Material* GetMaterial() const;
	R_Texture* GetTexture() const;
	R_Shader*  GetShader() const;
	
	void SetMaterial(R_Material* material);
	void SetTexture(R_Texture* texture);
	void SwapTexture(R_Texture* rTexture); //Set the texture without freeing the resource
	void SetShader(R_Shader* rShader);
																							// --- GET/SET COMPONENT MATERIAL VARIABLES
	
	Color GetMaterialColour();
	void SetMaterialColour(const Color& newColour);
	void SetMaterialColour(float r, float g, float b, float a);
		 
	void AddTextureMap(TextureMap textureMap, uint texId);
	void SetCurrentTextureMap(TextureMap textureMap);
		 
	bool UseDefaultTexture() const;
	void SetUseDefaultTexture(const bool& setTo);
																							//  --- GET RESOURCE TEXTURE DATA
	const char* GetTexturePath() const;
	const char* GetTextureFile() const;

	uint GetTextureID() const;
	void GetTextureSize(uint& width, uint& height);
		 
	void GetTextureInfo(uint& id, uint& width, uint& height, uint& depth, uint& bpp, uint& bytes, std::string& format, bool& compressed);

	uint32 const GetShaderProgramID();
	void SetShaderProgramID(uint32 ID);
	
public:
	//std::vector<R_Texture*>		textures;												// Will store all the textures that this component can have.
	//std::vector<uint>				texture_ids;											// Will store all the Texture Id's related with this Material Component.
	//uint							texture_maps[7];										// { 0, 0, 0, 0, 0, 0, 0 };
	//uint*							texture_maps;											// { 0, 0, 0, 0, 0, 0, 0 };
	//TEXTURE_MAP					current_map;

private:
	R_Material*	rMaterial;													// Currently used material.
	R_Texture*	rTexture;													// Currently used texture.
	R_Shader*	rShader;

	bool		useDefaultTex;
	bool		useAlbedoTex;
};

#endif // !__C_MATERIAL_H__