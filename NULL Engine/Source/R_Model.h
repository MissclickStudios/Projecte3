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
	ModelNode(const char* name, uint32 UID, uint32 parent_UID, Transform transform, uint32 mesh_UID, uint32 material_UID, uint32 texture_UID, std::string texture_name);

	bool Save(ParsonNode& root) const; 
	bool Load(const ParsonNode& root); 

	std::string name;																										// Name of the Model Node.
	uint32		uid;																										// UID of this Model Node.
	uint32		parent_uid;																									// UID of the parent Model Node.
	Transform	transform;																									// Transform of the Model Node.
	
	uint32		mesh_uid;																									// UID of the Mesh Resource (R_Mesh) of the Model Node.
	uint32		material_uid;																								// UID of the Material Resource (R_Material) of the Model Node.
	uint32		texture_uid;																								// UID of the Texture Resource (R_Texture) of the Model Node.
	std::string texture_name;
};

class R_Model : public Resource
{
public:
	R_Model();
	~R_Model();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& meta_root) const override;
	bool LoadMeta(const ParsonNode& meta_root) override;

public:
	std::vector<ModelNode>			model_nodes;
	std::map<uint32, std::string>	animations;

	ModelSettings			model_settings;
};

#endif // !__R_MODEL_H__