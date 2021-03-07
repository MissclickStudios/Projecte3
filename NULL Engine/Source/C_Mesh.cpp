#include "JSONParser.h"

#include "Application.h"
#include "M_Renderer3D.h"
#include "M_ResourceManager.h"

#include "R_Mesh.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Material.h"

#include "C_Mesh.h"

#define INVALID_BONE_ID 4294967295

C_Mesh::C_Mesh(GameObject* owner) : Component(owner, ComponentType::MESH),
rMesh			(nullptr),
skinnedMesh		(nullptr),
rootBone		(nullptr),
cAnimatorOwner	(nullptr),
showWireframe	(false),
showBoundingBox	(false)
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

	if (skinnedMesh != nullptr)
	{
		skinnedMesh->CleanUp();
		RELEASE(skinnedMesh);
	}

	boneMapping.clear();

	return ret;
}

bool C_Mesh::SaveState(ParsonNode& root) const
{
	bool ret = true;

	if (rMesh != nullptr)
	{
		root.SetNumber("Type", (double)GetType());

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
// --- MESH SKINNING METHODS
R_Mesh* C_Mesh::GetSkinnedMesh() const
{
	return skinnedMesh;
}

bool C_Mesh::RefreshSkinning()
{
	if (rMesh == nullptr)
		return false;
	
	bool isNewMesh = false;
	if (skinnedMesh == nullptr)
	{
		isNewMesh = true;
		skinnedMesh = (R_Mesh*)App->resourceManager->CreateResource(ResourceType::MESH, rMesh->GetAssetsPath());

		skinnedMesh->vertices.resize(rMesh->vertices.size());
		skinnedMesh->indices.resize(rMesh->indices.size());
		skinnedMesh->texCoords.resize(rMesh->texCoords.size());
		skinnedMesh->normals.resize(rMesh->normals.size());

		memcpy(&skinnedMesh->indices[0], &rMesh->indices[0], (rMesh->indices.size() * sizeof(uint)));
		memcpy(&skinnedMesh->texCoords[0], &rMesh->texCoords[0], (rMesh->texCoords.size() * sizeof(float)));
	}
	 
	if (!skinnedMesh->vertices.empty())
	{
		memset(&skinnedMesh->vertices[0], 0, (skinnedMesh->vertices.size() * sizeof(float)));
	}

	if (!skinnedMesh->normals.empty())
	{
		memset(&skinnedMesh->normals[0], 0, (skinnedMesh->normals.size() * sizeof(float)));
	}

	if (!skinnedMesh->texCoords.empty())
	{
		//memset(&skinnedMesh->texCoords[0], 0, (skinnedMesh->texCoords.size() * sizeof(float)));
	}

	if (isNewMesh)
	{
		RefreshBoneMapping();
		skinnedMesh->LoadSkinningBuffers(true);
	}
	
	return true;
}

void C_Mesh::AnimateMesh()
{
	if (rMesh == nullptr)
	{
		return;
	}
	if (skinnedMesh == nullptr)																						// Double Checking just in case.
	{
		RefreshSkinning();
	}

	GameObject* currentBone = nullptr;
	std::vector<float4x4> boneTransforms;
	boneTransforms.resize(rMesh->boneOffsets.size());
	for (auto bone = rMesh->boneMapping.begin(); bone != rMesh->boneMapping.end(); ++bone)
	{
		GameObject* currentBone = boneMapping[bone->first];

		float4x4 delta = currentBone->GetComponent<C_Transform>()->GetWorldTransform();
		delta = delta * this->GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Inverted();

		float4x4 delta = float4x4::identity;																												// --- Bone Global Transform
		delta = (cAnimatorOwner != nullptr) ? (cAnimatorOwner->GetComponent<C_Transform>()->GetWorldTransform().Inverted() * delta) : float4x4::identity;	//
		delta = (currentBone->GetComponent<C_Transform>()->GetWorldTransform() * delta);																	//
		delta = (this->GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Inverted() * delta);													// -------------------------

		delta = delta * rMesh->boneOffsets[bone->second];																									// --- Bone Transform Matrix
		boneTransforms[bone->second] = delta;																												// -------------------------
	}

	uint verticesSize = (rMesh->vertices.size() / 3);
	for (uint v = 0; v < verticesSize; ++v)																													// --- Iterating the mesh's verts.
	{
		for (uint b = 0; b < 4; ++b)																														// --- Iterating the verts bones.
		{
			uint boneID			= rMesh->boneIDs[(v * 4) + b];
			float boneWeight	= rMesh->boneWeights[(v * 4) + b];

			if (boneID == INVALID_BONE_ID || boneWeight == 0.0f)
			{
				continue;
			}

			float3 transform = boneTransforms[boneID].TransformPos(float3(&rMesh->vertices[v * 3]));														// --- Trnsfrm the original vert.

			if (!rMesh->vertices.empty())
			{
				skinnedMesh->vertices[(v * 3)]		+= transform.x * boneWeight;
				skinnedMesh->vertices[(v * 3) + 1]	+= transform.y * boneWeight;
				skinnedMesh->vertices[(v * 3) + 2]	+= transform.z * boneWeight;
			}
			if (!rMesh->normals.empty())
			{
				transform = boneTransforms[boneID].TransformPos(float3(&rMesh->normals[v * 3]));

				skinnedMesh->normals[(v * 3)]		+= transform.x * boneWeight;
				skinnedMesh->normals[(v * 3) + 1]	+= transform.y * boneWeight;
				skinnedMesh->normals[(v * 3) + 2]	+= transform.z * boneWeight;
			}
		}
	}

	skinnedMesh->LoadSkinningBuffers();
}

void C_Mesh::RefreshBoneMapping()
{
	boneMapping.clear();

	if (rootBone == nullptr)
	{
		return;
	}

	boneMapping.emplace(rootBone->GetName(), rootBone);
	rootBone->GetAllChilds(boneMapping);
}

void C_Mesh::SetRootBone(GameObject* rootBone)
{
	this->rootBone = rootBone;
}

void C_Mesh::SetAnimatorOwner(GameObject* cAnimatorOwner)
{
	this->cAnimatorOwner = cAnimatorOwner;
}

// --- RESOURCE MESH METHODS
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

	return "[NONE]";
}

const char* C_Mesh::GetMeshFile() const
{
	if (rMesh != nullptr)
	{
		return rMesh->GetAssetsFile();
	}

	return "[NONE]";
}

void C_Mesh::SetMeshPath(const char* path)
{
	if (rMesh != nullptr)
	{
		rMesh->SetAssetsPath(path);
	}
}

// --- C_MESH DEBUG METHODS
void C_Mesh::GetMeshData(uint& numVertices, uint& numNormals, uint& numTexCoords, uint& numIndices, uint& numBones)
{
	if (rMesh != nullptr)
	{
		numVertices		= rMesh->vertices.size();
		numNormals		= rMesh->normals.size();
		numTexCoords	= rMesh->texCoords.size();
		numIndices		= rMesh->indices.size();
		numBones		= rMesh->boneMapping.size();
	}
}

void C_Mesh::GetBoundingBoxVertices(math::float3* bbVertices) const
{
	if (rMesh != nullptr)
	{
		rMesh->aabb.GetCornerPoints(bbVertices);
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

bool C_Mesh::GetShowWireframe() const
{
	return showWireframe;
}

bool C_Mesh::GetShowBoundingBox() const
{
	return showBoundingBox;
}

void C_Mesh::SetDrawVertexNormals(bool setTo)
{
	if (rMesh != nullptr)
	{
		rMesh->drawVertexNormals = setTo;
	}
}

void C_Mesh::SetDrawFaceNormals(bool setTo)
{
	if (rMesh != nullptr)
	{
		rMesh->drawFaceNormals = setTo;
	}
}

void C_Mesh::SetShowWireframe(bool setTo)
{
	showWireframe = setTo;
}

void C_Mesh::SetShowBoundingBox(bool setTo)
{
	showBoundingBox = setTo;
	this->GetOwner()->show_bounding_boxes = setTo;
}