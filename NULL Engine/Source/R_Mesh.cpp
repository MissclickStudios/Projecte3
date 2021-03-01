#include "OpenGL.h"

#include "VariableTypedefs.h"

#include "JSONParser.h"

#include "Application.h"
#include "M_Renderer3D.h"

#include "R_Mesh.h"

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

R_Mesh::R_Mesh() : Resource(ResourceType::MESH)
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
	glDeleteBuffers(1, (GLuint*)&VAO);
	glDeleteBuffers(1, (GLuint*)&VBO);
	glDeleteBuffers(1, (GLuint*)&NBO);
	glDeleteBuffers(1, (GLuint*)&TBO);
	glDeleteBuffers(1, (GLuint*)&IBO);

	// --- Clear vectors
	vertices.clear();
	normals.clear();
	texCoords.clear();
	indices.clear();

	//bones.clear();

	return true;
}

bool R_Mesh::SaveMeta(ParsonNode& metaRoot) const
{
	bool ret = true;

	ParsonArray contained_array = metaRoot.SetArray("ContainedResources");

	ParsonNode settings = metaRoot.SetNode("ImportSettings");
	meshSettings.Save(settings);

	return ret;
}

bool R_Mesh::LoadMeta(const ParsonNode& metaRoot)
{
	bool ret = true;



	return ret;
}

// --- R_MESH METHODS
void R_Mesh::LoadBuffers()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	if (!vertices.empty())
	{
		glGenBuffers(1, (GLuint*)&VBO);																			// Generates the Vertex Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, VBO);																		// Binds VBO with the GL_ARRAY_BUFFER binding point (target)
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);			// Inits the data stored inside VBO and specifies how it will be accessed.
	}

	if (!indices.empty())
	{
		glGenBuffers(1, (GLuint*)&IBO);																			// Generates the Index Buffer Object
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);																// Binds IBO with the GL_ARRAY_BUFFER binding point (target)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);		// Inits the data stored inside IBO and specifies how it will be accessed.

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	if (!texCoords.empty())
	{
		glGenBuffers(1, (GLuint*)&TBO);
		glBindBuffer(GL_ARRAY_BUFFER, TBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
	}

	if (!normals.empty())
	{
		glGenBuffers(1, (GLuint*)&NBO);
		glBindBuffer(GL_ARRAY_BUFFER, NBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), &normals[0], GL_STATIC_DRAW);
	
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);
	}
}

void R_Mesh::SwapBonesToVertexArray()
{
	/*for (auto bone = bones.cbegin(); bone != bones.cend(); ++bone)
	{

	}*/
	
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