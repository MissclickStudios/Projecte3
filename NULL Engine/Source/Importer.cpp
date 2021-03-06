#include <string>
#include <vector>

#include "VariableTypedefs.h"

#include "Application.h"
#include "M_FileSystem.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "R_Model.h"
#include "R_Texture.h"

#include "I_Scenes.h"
#include "I_Meshes.h"
#include "I_Materials.h"
#include "I_Textures.h"

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

	if (rModel == nullptr)
	{
		return false;
	}

	return true;
}

bool Importer::ImportMesh(const char* buffer, R_Mesh* rMesh)
{
	//Importer::Meshes::Import(buffer)
	
	return false;
}

bool Importer::ImportTexture(const char* buffer, uint size, R_Texture* rTexture)
{
	bool success = Importer::Textures::Import(buffer, size, rTexture);

	/*if (rTexture != nullptr && rTexture->GetTextureID() != 0)
	{
		//App->scene->ApplyNewTextureToSelectedGameObject(rTexture->GetUID());					// Put on hold until the project panel is functional and in place.
	}
	else
	{
		return false;
		
		LOG("[ERROR] Importer: Could not import the dropped texture!");
		RELEASE(rTexture);
		return 0;
	}*/

	return success;
}