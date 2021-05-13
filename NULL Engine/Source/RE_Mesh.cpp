#include "C_Mesh.h"
#include "C_Material.h"

#include "RE_Mesh.h"

RE_Mesh::RE_Mesh(float4x4* transform, C_Mesh* cMesh, C_Material* cMaterial) : Renderer(RendererType::MESH, transform),
cMesh		(cMesh),
cMaterial	(cMaterial)
{

}

RE_Mesh::~RE_Mesh()
{

}

bool RE_Mesh::Render()
{
	return true;
}

bool RE_Mesh::CleanUp()
{
	return true;
}