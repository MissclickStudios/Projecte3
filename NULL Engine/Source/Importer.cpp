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

using namespace Importer;																						// Not a good thing to do but it will be employed only inside this .cpp file.

void Importer::InitializeImporters()
{

}

void Importer::ShutDownImporters()
{

}

bool Importer::ImportScene(const char* buffer, uint size, R_Model* r_model)
{
	Importer::Scenes::Import(buffer, size, r_model);

	if (r_model == nullptr)
	{
		return false;
	}

	return true;
}

bool Importer::ImportMesh(const char* buffer, R_Mesh* r_mesh)
{
	//Importer::Meshes::Import(buffer)
	
	return false;
}

bool Importer::ImportTexture(const char* buffer, uint size, R_Texture* r_texture)
{
	bool success = Importer::Textures::Import(buffer, size, r_texture);

	/*if (r_texture != nullptr && r_texture->GetTextureID() != 0)
	{
		//App->scene->ApplyNewTextureToSelectedGameObject(r_texture->GetUID());					// Put on hold until the project panel is functional and in place.
	}
	else
	{
		return false;
		
		LOG("[ERROR] Importer: Could not import the dropped texture!");
		RELEASE(r_texture);
		return 0;
	}*/

	return success;
}