#ifndef __R_MATERIAL_H__
#define __R_MATERIAL_H__

#include "Color.h"

#include "Resource.h"
#include "MaterialSettings.h"

class ParsonNode;

typedef unsigned int uint;

enum class TEXTURE_TYPE									// The enum values are set according to the values of Assimp's aiTextureType enum.
{
	NONE			= 0x0,								// 0x0 = aiTextureType_NONE			--> No texture. The value will be used for all material properties non-related to textures.
	DIFFUSE			= 0x1,								// 0x1 = aiTextureType_DIFFUSE		--> Tex. will be combined with the diffuse lighting equation's result.
	SPECULAR		= 0x2,								// 0x2 = aiTextureType_SPECULAR		--> Tex. will be combined with the specular lighting equation's result.
	AMBIENT			= 0x3,								// 0x3 = aiTextureType_AMBIENT		--> Tex. will be combined with the ambient lighting equation's result.
	EMISSIVE		= 0x4,								// 0x4 = aiTextureType_EMISSIVE		--> Tex. will be added to the lighting equation's result. Not influenced by incoming light.
	HEIGHT			= 0x5,								// 0x5 = aiTextureType_HEIGHT		--> Tex. is a height map. Convention: Higher gray-scale values stand for higher elevations.
	NORMALS			= 0x6,								// 0x6 = aiTextureType_NORMALS		--> Tex. is a tangent space normal map. Tangent-Space conventions will not be distinguished.
	SHININESS		= 0x7,								// 0x7 = aiTextureType_SHININESS	--> Tex. defines the Glossiness of the material. G = Exponent of the specular lighting equation.
	OPACITY			= 0x8,								// 0x8 = aiTextureType_OPACITY		--> Tex. defines the per-pixel opacity. Convention: White = Opaque, Black = Transparent.
	DISPLACEMENT	= 0x9,								// 0x9 = aiTextureType_DISPLACEMENT	--> Tex. defines vertex displacements. Higher color values = Higher vertex displacements.
	LIGHTMAP		= 0xA,								// 0xA = aiTextureType_LIGHTMAP		--> Tex. contains a scaling value for the final color value of a pixel. Intensity != inc. light.
	REFLECTION		= 0xB,								// 0xB = aiTextureType_REFLECTION	--> Tex. contains the color of a perfect mirror reflection. Almost never used im real-time Apps.
	UNKNOWN			= 0xC								// 0xC = aiTextureType_UNKNOWN		--> Tex. ref. that does not match any of the above. Imported but excluded from postprocessing.
};

struct MaterialData
{
	MaterialData();
	MaterialData(TEXTURE_TYPE type, uint texture_uid, std::string texture_assets_path);
	
	void CleanUp();
	
	TEXTURE_TYPE	type;
	uint			texture_uid;
	std::string		texture_assets_path;
};

class R_Material : public Resource
{
public:
	R_Material();
	~R_Material();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& meta_root) const override;
	bool LoadMeta(const ParsonNode& meta_root) override;

public:
	std::vector<MaterialData>	materials;									// In the end the amount of materials that will be used is "constant". It could be an array as mat[7] = { 0, 0,... };
	Color					diffuse_color;

private:
	MaterialSettings		material_settings;
};

#endif // !__R_MATERIAL_H__