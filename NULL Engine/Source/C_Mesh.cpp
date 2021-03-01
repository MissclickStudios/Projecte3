#include "JSONParser.h"

#include "Application.h"
#include "M_Renderer3D.h"
#include "M_ResourceManager.h"

#include "R_Mesh.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Material.h"

#include "C_Mesh.h"

C_Mesh::C_Mesh(GameObject* owner) : Component(owner, ComponentType::MESH),
rMesh(nullptr),
showWireframe(false),
showBoundingBox(false)
{
}

C_Mesh::~C_Mesh()
{

}

bool C_Mesh::Update()
{
	bool ret = true;

	return ret;
}

bool C_Mesh::CleanUp()
{
	bool ret = true;

	if (rMesh != nullptr)
	{
		App->resourceManager->FreeResource(rMesh->GetUID());
		rMesh = nullptr;
	}

	return ret;
}

bool C_Mesh::SaveState(ParsonNode& root) const
{
	bool ret = true;

	if (rMesh != nullptr)
	{
		root.SetNumber("Type", (uint)GetType());
		root.SetNumber("UID", rMesh->GetUID());
		root.SetString("Name", rMesh->GetAssetsFile());
		root.SetString("Path", rMesh->GetLibraryPath());
		root.SetString("File", rMesh->GetLibraryFile());
		root.SetBool("ShowWireframe", showWireframe);
		root.SetBool("ShowBoundingBox", showBoundingBox);
	}

	return ret;
}

bool C_Mesh::LoadState(ParsonNode& root)
{
	bool ret = true;

	rMesh = nullptr;

	std::string assetsPath = ASSETS_MODELS_PATH + std::string(root.GetString("Name"));
	App->resourceManager->AllocateResource((uint32)root.GetNumber("UID"), assetsPath.c_str());
	
	rMesh = (R_Mesh*)App->resourceManager->RequestResource((uint32)root.GetNumber("UID"));
	showWireframe = root.GetBool("ShowWireframe");
	showBoundingBox = root.GetBool("ShowBoundingBox");

	if (rMesh == nullptr)
	{
		LOG("[ERROR] Loading Scene: Could not find Mesh %s with UID: %u! Try reimporting the model.", root.GetString("File"), (uint32)root.GetNumber("UID"));
	}

	return ret;
}

// --- C_MESH METHODS ---
R_Mesh* C_Mesh::GetMesh() const
{
	return rMesh;
}

void C_Mesh::SetMesh(R_Mesh* rMesh)
{
	if (this->rMesh != nullptr)
	{
		App->resourceManager->FreeResource(this->rMesh->GetUID());
	}
	
	this->rMesh = rMesh;
}

const char* C_Mesh::GetMeshPath() const
{
	if (rMesh != nullptr)
	{
		return rMesh->GetAssetsPath();
	}

	return "NONE";
}

const char* C_Mesh::GetMeshFile() const
{
	if (rMesh != nullptr)
	{
		return rMesh->GetAssetsFile();
	}

	return "NONE";
}

void C_Mesh::SetMeshPath(const char* path)
{
	if (rMesh != nullptr)
	{
		rMesh->SetAssetsPath(path);
	}
}

void C_Mesh::GetMeshData(uint& numVertices, uint& numNormals, uint& numTexCoords, uint& numIndices, uint& numBones)
{
	if (rMesh != nullptr)
	{
		numVertices = rMesh->vertices.size();
		numNormals = rMesh->normals.size();
		numTexCoords = rMesh->texCoords.size();
		numIndices = rMesh->indices.size();

		numBones = rMesh->boneMapping.size();
	}
}

bool C_Mesh::GetDrawVertexNormals() const
{
	if (rMesh != nullptr)
	{
		return rMesh->drawVertexNormals;
	}

	return false;
}

bool C_Mesh::GetDrawFaceNormals() const
{
	if (rMesh != nullptr)
	{
		return rMesh->drawFaceNormals;
	}

	return false;
}

void C_Mesh::SetDrawVertexNormals(const bool& setTo)
{
	if (rMesh != nullptr)
	{
		rMesh->drawVertexNormals = setTo;
	}
}

void C_Mesh::SetDrawFaceNormals(const bool& setTo)
{
	if (rMesh != nullptr)
	{
		rMesh->drawFaceNormals = setTo;
	}
}

void C_Mesh::GetBoundingBoxVertices(math::float3* bbVertices) const
{
	if (rMesh != nullptr)
	{
		rMesh->aabb.GetCornerPoints(bbVertices);
	}
}

bool C_Mesh::GetShowWireframe() const
{
	return showWireframe;
}

void C_Mesh::SetShowWireframe(const bool& setTo)
{
	showWireframe = setTo;
}

bool C_Mesh::GetShowBoundingBox() const
{
	return showBoundingBox;
}

void C_Mesh::SetShowBoundingBox(const bool& setTo)
{
	showBoundingBox = setTo;
	this->GetOwner()->show_bounding_boxes = setTo;
}