#ifndef __RE_MESH_H__
#define __RE_MESH_H__

#include "Renderer.h"

class C_Mesh;
class C_Material;

class RE_Mesh : public Renderer
{
public:
	RE_Mesh(C_Transform* cTransform, C_Mesh* cMesh, C_Material* cMaterial);
	~RE_Mesh();

	bool Render() override;
	bool CleanUp() override;

private:
	void RenderVertexNormals		(const R_Mesh* rMesh);
	void RenderFaceNormals			(const R_Mesh* rMesh);

	void GetFaces					(const R_Mesh* rMesh, std::vector<Triangle>& vertexFaces, std::vector<Triangle>& normalFaces);

	void ApplyDebugParameters		();
	void ClearDebugParameters		();
	
	void ApplyTextureAndMaterial	();
	void ClearTextureAndMaterial	();

	void RenderOutline				(R_Mesh* rMesh);

	void ApplyShader				();
	uint32 SetDefaultShader			(C_Material* cMaterial);
	void ClearShader				();

private:
	C_Transform*	cTransform;
	C_Mesh*			cMesh;
	C_Material*		cMaterial;

	std::vector<float4x4>* boneTransforms;
};

#endif // !__RE_MESH_H__