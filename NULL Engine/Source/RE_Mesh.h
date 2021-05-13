#ifndef __RE_MESH_H__
#define __RE_MESH_H__

#include "Renderer.h"

class C_Mesh;
class C_Material;

class RE_Mesh : public Renderer
{
public:
	RE_Mesh(float4x4* transform, C_Mesh* cMesh, C_Material* cMaterial);
	~RE_Mesh();

	bool Render() override;
	bool CleanUp() override;

private:
	C_Mesh*		cMesh;
	C_Material* cMaterial;
};

#endif // !__RE_MESH_H__