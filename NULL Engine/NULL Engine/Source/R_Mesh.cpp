#include "Resource.h"
#include "OpenGL.h"
#include "Profiler.h"

#include "VariableTypedefs.h"

#include "JSONParser.h"

#include "Application.h"
#include "M_Renderer3D.h"


#include "R_Mesh.h"

#include "MemoryManager.h"

#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

R_Mesh::R_Mesh() : Resource(ResourceType::MESH),
drawVertexNormals	(false),
drawFaceNormals		(false),
hasBones			(false)
{
	
}

R_Mesh::~R_Mesh()
{

}

bool R_Mesh::CleanUp()
{
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

	boneIDs.clear();
	boneWeights.clear();
	boneOffsets.clear();
	boneMapping.clear();

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
void R_Mesh::LoadStaticBuffers()
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

	glBindVertexArray(0);
}

void R_Mesh::LoadSkinningBuffers(bool initStatic)
{
	OPTICK_CATEGORY("Load Skinning Buffers", Optick::Category::Animation)
	
	if (initStatic)
	{
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		if (!vertices.empty())
		{
			glGenBuffers(1, (GLuint*)&VBO);
		}
		if (!indices.empty())
		{
			glGenBuffers(1, (GLuint*)&IBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);
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
		}
	}

	glBindVertexArray(VAO);

	if (!vertices.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, (GLuint)VBO);
		glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(float)), &vertices[0], GL_DYNAMIC_DRAW);			// GL_DYNAMIC_DRAW or GL_STREAM_DRAW? WHAT ENTAILS MANY TIMES OF USAGE?
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3 * sizeof(float)), (void*)0);
		glEnableVertexAttribArray(0);
	}
	if (!normals.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, (GLuint)NBO);
		glBufferData(GL_ARRAY_BUFFER, (normals.size() * sizeof(float)), &normals[0], GL_DYNAMIC_DRAW);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (3 * sizeof(float)), (void*)0);
		glEnableVertexAttribArray(2);
	}

	glBindVertexArray(0);
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