#include <string>
#include <vector>

#include "VariableTypedefs.h"

#include "Application.h"
#include "M_FileSystem.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "R_Model.h"
#include "R_Texture.h"
#include "R_Shader.h"
#include "R_Script.h"

#include "I_Scenes.h"
#include "I_Meshes.h"
#include "I_Materials.h"
#include "I_Textures.h"
#include "I_Shaders.h"
//#include "I_Scripts.h"
#include "I_NavMesh.h"

#include "Importer.h"

#include "MemoryManager.h"

using namespace Importer;																						// Not a good thing to do but it will be employed only inside this .cpp file.

void Importer::InitializeImporters()
{

}

void Importer::ShutDownImporters()
{

}

bool Importer::ImportScene(const char* buffer, uint size, R_Model* rModel)
{
	Importer::Scenes::Import(buffer, size, rModel);

	return (rModel != nullptr);
}

bool Importer::ImportMesh(const char* buffer, R_Mesh* rMesh)
{
	//Importer::Meshes::Import(buffer)
	
	return false;
}

bool Importer::ImportTexture(const char* buffer, uint size, R_Texture* rTexture)
{
	FindAndForceUID((Resource*)rTexture);
	
	bool success = Importer::Textures::Import(buffer, size, rTexture);

	return (success && rTexture != nullptr);
}

bool Importer::ImportShader(const char* buffer, R_Shader* rShader)
{
	FindAndForceUID((Resource*)rShader);
	
	bool success = Importer::Shaders::Import(buffer, rShader);

	return (success && rShader != nullptr);
}

bool Importer::ImportParticles(const char* buffer, R_ParticleSystem* rParticles)
{
	FindAndForceUID((Resource*)rParticles);
	
	/*bool success = Importer::Particles::Import(buffer, rParticles);*/

	return /*(success && rParticles != nullptr)*/ true;
}

bool Importer::ImportScript(const char* buffer, R_Script* rScript)
{
	FindAndForceUID((Resource*)rScript);

	//bool success = Importer::Scripts::Import(rScript->GetAssetsPath(), buffer, );
	
	return /*(success && rScript != nullptr)*/ true;
}

bool Importer::ImportNavMesh(const char* buffer, R_NavMesh* rNavMesh)
{
	FindAndForceUID((Resource*)rNavMesh);

	bool success = Importer::NavMesh::Import(buffer, rNavMesh);

	return (success && rNavMesh != nullptr);
}

void Importer::FindAndForceUID(Resource* resource)
{
	if (resource == nullptr)
	{
		LOG("[ERROR] Importer: Could not Find and Force UID! Error: Given Resource* was nullptr");
		return;
	}

	uint32 forcedUID = App->resourceManager->GetForcedUIDFromMeta(resource->GetAssetsPath());
	if (forcedUID != 0)
	{
		resource->ForceUID(forcedUID);
	}
}