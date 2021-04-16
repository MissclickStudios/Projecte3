#ifndef __C_MESH_H__
#define __C_MESH_H__

#include "Component.h"

namespace math
{
	class float3;
}

class ParsonNode;
class GameObject;
class R_Mesh;

class NULL_API C_Mesh : public Component
{
public:
	C_Mesh(GameObject* owner);
	~C_Mesh();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::MESH; }															// Required for templated methods such as GetComponent().

public:																																// --- RESOURCE MESH METHODS
	R_Mesh*		GetMesh					() const;																					// If there is no rMesh it returns nullptr.
	void		SetMesh					(R_Mesh* rMesh);																			// rMesh will be normally set when a model is imported.

	const char* GetMeshPath				() const;																					// 
	const char* GetMeshFile				() const;																					// 
	void		SetMeshPath				(const char* path);																			// 

public:																																// --- MESH SKINNING METHODS
	R_Mesh*		GetSkinnedMesh			() const;
	void		GetBoneMapping			(std::map<std::string, GameObject*>& boneMapping);
	
	bool		RefreshSkinning			();																							// 
	void		AnimateMesh				();																							// 

	void		RefreshBoneMapping		();
	void		SetRootBone				(GameObject* rootBone);
	void		SetAnimatorOwner		(GameObject* cAnimatorOwner);																// TMP. Ambiguous name. Change Later.

public:																																// --- C_MESH DEBUG METHODS
	void		GetMeshData				(uint& numVertices, uint& numNormals, uint& numTexCoords, uint& numIndices, uint& numBones);
	void		GetBoundingBoxVertices	(math::float3* bbVertices) const;															// TODO: Kinda dirty, should be done elsewhere (?).

	bool		GetDrawVertexNormals	() const;
	bool		GetDrawFaceNormals		() const;
	bool		GetShowWireframe		() const;
	bool		GetShowBoundingBox		() const;
	
	void		SetDrawVertexNormals	(bool setTo);
	void		SetDrawFaceNormals		(bool setTo);
	void		SetShowWireframe		(bool setTo);
	void		SetShowBoundingBox		(bool setTo);

private:
	R_Mesh*		rMesh;
	R_Mesh*		skinnedMesh;

	GameObject* rootBone;
	GameObject* cAnimatorOwner;

	std::vector<float4x4>				boneTransforms;
	std::map<std::string, GameObject*>	boneMapping;
	
private:																															// --- COMPONENT MESH DEBUG VARIABLES
	bool	showWireframe;
	bool	showBoundingBox;
};

#endif // !__C_MESH_H__