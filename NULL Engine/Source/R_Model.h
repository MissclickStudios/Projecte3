#ifndef __R_MODEL_H__
#define __R_MODEL_H__

#include <string>
#include <map>

#include "MathGeoTransform.h"

#include "Resource.h"
#include "ModelSettings.h"

class ParsonNode;

typedef unsigned __int32 uint32;

struct ModelNode
{
	ModelNode();
	ModelNode(const char* name, uint32 UID, uint32 parentUID, Transform transform, uint32 meshUID, uint32 materialUID, uint32 textureUID, uint32 shaderUID, std::string textureName);

	bool Save(ParsonNode& root) const;
	bool Load(const ParsonNode& root);

	std::string		name;																									// Name of the Model Node.
	uint32			uid;																									// UID of this Model Node.
	uint32			parentUID;																								// UID of the parent Model Node.
	Transform		transform;																								// Transform of the Model Node.
	
	uint32			meshUID;																								// UID of the Mesh Resource (R_Mesh) of the Model Node.
	uint32			materialUID;																							// UID of the Material Resource (R_Material) of the Model Node.
	uint32			shaderUID;
	uint32			textureUID;																								// UID of the Texture Resource (R_Texture) of the Model Node.
	std::string		textureName;
};

class MISSCLICK_API R_Model : public Resource
{
public:
	R_Model();
	~R_Model();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

	static inline ResourceType GetType() { return ResourceType::MODEL; }

public:
	std::vector<ModelNode>			modelNodes;
	std::map<uint32, std::string>	animations;

	ModelSettings modelSettings;
};

#endif // !__R_MODEL_H__