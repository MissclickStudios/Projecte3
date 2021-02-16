#include "OpenGL.h"

#include "VariableTypedefs.h"

#include "JSONParser.h"

#include "Application.h"
#include "M_Renderer3D.h"

#include "R_Mesh.h"

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

R_Mesh::R_Mesh() : Resource(RESOURCE_TYPE::MESH), 
VBO					(0),																								// Initializing the buffers.
NBO					(0), 																								// 
TBO					(0), 																								//
IBO					(0), 																								// -------------------------
draw_vertex_normals	(false), 
draw_face_normals	(false)
{
	
}

R_Mesh::~R_Mesh()
{

}

bool R_Mesh::CleanUp()
{
	bool ret = true;
	
	App->renderer->DeleteFromMeshRenderers(this);

	// --- Delete Buffers
	glDeleteBuffers(1, (GLuint*)&VBO);
	glDeleteBuffers(1, (GLuint*)&NBO);
	glDeleteBuffers(1, (GLuint*)&TBO);
	glDeleteBuffers(1, (GLuint*)&IBO);

	// --- Clear vectors
	vertices.clear();
	normals.clear();
	tex_coords.clear();
	indices.clear();

	bones.clear();

	return true;
}

bool R_Mesh::SaveMeta(ParsonNode& meta_root) const
{
	bool ret = true;

	ParsonArray contained_array = meta_root.SetArray("ContainedResources");

	ParsonNode settings = meta_root.SetNode("ImportSettings");
	mesh_settings.Save(settings);

	return ret;
}

bool R_Mesh::LoadMeta(const ParsonNode& meta_root)
{
	bool ret = true;



	return ret;
}

// --- R_MESH METHODS
void R_Mesh::LoadBuffers()
{
	if (!vertices.empty())
	{
		glGenBuffers(1, (GLuint*)&VBO);																			// Generates the Vertex Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, VBO);																		// Binds VBO with the GL_ARRAY_BUFFER binding point (target)
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);			// Inits the data stored inside VBO and specifies how it will be accessed.
	}

	if (!normals.empty())
	{
		glGenBuffers(1, (GLuint*)&NBO);
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), &normals[0], GL_STATIC_DRAW);
	}

	if (!tex_coords.empty())
	{
		glGenBuffers(1, (GLuint*)&TBO);
		glBindBuffer(GL_ARRAY_BUFFER, TBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
	}

	if (!indices.empty())
	{
		glGenBuffers(1, (GLuint*)&IBO);																			// Generates the Index Buffer Object
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);																// Binds IBO with the GL_ARRAY_BUFFER binding point (target)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);		// Inits the data stored inside IBO and specifies how it will be accessed.
	}
}

void R_Mesh::SwapBonesToVertexArray()
{
	for (auto bone = bones.cbegin(); bone != bones.cend(); ++bone)
	{

	}
	
	// Bone Weights
	//bone_weights.push_back(BoneWeight(bone_id, weight));

	// Bone offsets
	//bone_offsets.push_back(offset_matrix);

	// Bone Mapping
	//bone_mapping.emplace(bone_name, bone_id);
	
	// Bone transforms

}

AABB R_Mesh::GetAABB() const
{
	return aabb;
}

void R_Mesh::SetAABB()
{
	aabb.SetNegativeInfinity();
	aabb.Enclose((float3*)&vertices[0], vertices.size() / 3);
}