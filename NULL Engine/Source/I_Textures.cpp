// ----------------------------------------------------
// I_Textures.cpp --- Importing textures from files.
// Interface class between engine and DevIL.
// ----------------------------------------------------

#include "OpenGL.h"
#include "DevIL.h"

#include "VariableTypedefs.h"
#include "Log.h"

#include "Random.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "R_Texture.h"

#include "I_Textures.h"

#include "MemoryManager.h"

#pragma comment (lib, "Source/Dependencies/DevIL/libx86/DevIL.lib")
#pragma comment (lib, "Source/Dependencies/DevIL/libx86/ILU.lib")
#pragma comment (lib, "Source/Dependencies/DevIL/libx86/ILUT.lib")

using namespace Importer::Textures;																				// Not a good thing to do but it will be used sparsely and only inside this .cpp

void Importer::Textures::Init()
{
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION || ilGetInteger(ILU_VERSION_NUM) < ILU_VERSION || ilGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION)
	{
		LOG("[ERROR] DevIL Version does not match with lib version.");
	}

	LOG("[STATUS] Initializing DevIL");
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
}

void Importer::Textures::CleanUp()
{
	LOG("[STATUS] Shutting down DevIL");
	ilShutDown();
}

bool Importer::Textures::Import(const char* buffer, uint size, R_Texture* r_texture)
{
	bool success = false;

	if (r_texture == nullptr)
	{
		LOG("[ERROR] Could not Import Texture! Error: R_Texture* was nullptr.");
		return false;
	}
	
	std::string error_string = "[ERROR] Could not Import Texture { " + std::string(r_texture->GetAssetsFile()) + " }";

	if (buffer == nullptr)
	{
		LOG("%s! Error: Buffer was nullptr.", error_string.c_str());
		return false;
	}
	if (size == 0)
	{
		LOG("%s! Error: Buffer Size was 0.", error_string.c_str());
		return false;
	}

	LOG("[STATUS] Importer: Importing Texture { %s }", r_texture->GetAssetsPath());

	success = ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size);															// Only loading inside DevIL buffers. Needs to be saved after.
	if (!success)																											// Import settings will be applied at Importer::Textures::Load().
	{
		LOG("%s! Error: ilLoadL() Error [%s]", error_string.c_str(), iluErrorString(ilGetError()));
		return false;
	}

	return success;
}

uint Importer::Textures::Save(const R_Texture* r_texture, char** buffer)
{
	uint written = 0;
	
	if (r_texture == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save Texture! Error: R_Texture* was nullptr.");
		return 0;
	}
	
	std::string error_string = "[ERROR] Importer: Could not Save Texture { " + std::string(r_texture->GetAssetsFile()) + " }";

	std::string directory	= TEXTURES_PATH;																					// --- Getting the path to which save the texture.
	std::string file		= std::to_string(r_texture->GetUID()) + std::string(TEXTURES_EXTENSION);							// 
	std::string full_path	= directory + file;																					// -----------------------------------------------

	ilEnable(IL_FILE_OVERWRITE);																								// Allowing DevIL to overwrite existing files.
	ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);																						// Choosing a specific DXT compression.

	ILuint size = (ilGetInteger(IL_IMAGE_TYPE) != IL_DDS) ? ilSaveL(IL_DDS, nullptr, 0) : ilGetInteger(IL_IMAGE_SIZE_OF_DATA);	// Getting the size required by the texture.
	if (size > 0)
	{
		ILubyte* data = new ILubyte[size];																						// Allocating the required memory to the data buffer.

		if (ilSaveL(IL_DDS, data, size) > 0)																					// ilSaveL() saves the current image with the specified type.
		{	
			*buffer = (char*)data;
			written = App->file_system->Save(full_path.c_str(), *buffer, size, false);											// Saving the texture throught the file system.
			if (written > 0)
			{	
				LOG("[IMPORTER] Importer: Successfully Saved { %s } in { %s }", file.c_str(), directory.c_str());
			}
			else
			{
				*buffer = nullptr;
				LOG("%s! Error: File System could not Write the File.", error_string.c_str());
			}
		}
		else
		{
			LOG("%s! Error: ilSaveL() Error [%s]", error_string.c_str(), iluErrorString(ilGetError()));
		}

		//RELEASE_ARRAY(data);																									// Dealt with in Resource Manager.
	}
	else
	{
		LOG("%s! ilSaveL() Error: %s", error_string.c_str(), iluErrorString(ilGetError()));
	}

	return written;
}

bool Importer::Textures::Load(const char* buffer, const uint size, R_Texture* r_texture)
{
	bool ret = true;
	
	if (r_texture == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Texture from Library! Error: R_Texture* was nullptr.");
		return false;
	}

	std::string error_string = "[ERROR] Importer: Could not Load Texture { " + std::string(r_texture->GetAssetsFile()) + " } from Library";

	if (buffer == nullptr)
	{
		LOG("%s! Error: Buffer was nullptr.", error_string.c_str());
		return false;
	}
	if (size == 0)
	{
		LOG("%s! Error: Buffer Size was 0", error_string.c_str());
		return false;
	}

	ILuint il_image = 0;																				// Will be used to generate, bind and delete the buffers created by DevIL.
	ilGenImages(1, &il_image);																			// DevIL's buffers work pretty much the same way as OpenGL's do.
	ilBindImage(il_image);																				// The first step is to generate a buffer and then bind it.

	bool success = ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size);									// ilLoadL() loads a texture from some buffer data. size == 0 = no bounds check.
	if (success)																						// --- When type is IL_TYPE_UNKNOWN, DevIL will try to find the type on it's own.
	{	
		uint color_channels = ilGetInteger(IL_IMAGE_CHANNELS);
		if (color_channels == 3)
		{
			success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);											// ilConvertImage() will convert the image to the given format and type.
		}
		else if (color_channels == 4)
		{
			success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);										// ilConvertImage() will return false if the system cannot store the image with
		}																								// its new format or the operation is invalid (no bound img. or invalid identifier).
		else
		{
			LOG("[WARNING] Texture has < 3 color channels! Path: %s", r_texture->GetAssetsPath());
		}

		if (success)
		{
			//ilutGLBindTexImage();

			ILinfo il_info;
			iluGetImageInfo(&il_info);

			if (il_info.Origin == IL_ORIGIN_UPPER_LEFT)
			{
				iluFlipImage();
			}
			
			uint width		= il_info.Width;																				// --------------------- Getting the imported texture's data.
			uint height		= il_info.Height;																				// 
			uint depth		= il_info.Depth;																				// 
			uint bpp		= il_info.Bpp;																					// 
			uint size		= il_info.SizeOfData;																			// 
			uint format		= il_info.Format;																				// Internal format will be forced to be the same as format.
			uint target		= (uint)GL_TEXTURE_2D;																			// 
			int wrapping	= (int)GL_REPEAT;																				// 
			int filter		= (int)GL_LINEAR;																				// ----------------------------------------------------------

			uint tex_id = Utilities::CreateTexture(ilGetData(), width, height, target, wrapping, filter, format, format);	// Creates an OpenGL texture with the given data and parameters.
			if (tex_id != 0)																								// If tex_id == 0, then it means the tex. could not be created.
			{
				r_texture->SetTextureData(tex_id, width, height, depth, bpp, size, (TEXTURE_FORMAT)format);
			}
			else
			{
				LOG("%s! Error: Could not get texture ID.", error_string.c_str());
				ret = false;
			}
		}
		else
		{
			LOG("%s! Error: ilConvertImage() Error [%s]", error_string.c_str(), iluErrorString(ilGetError()));
			ret = false;
		}
	}
	else
	{
		LOG("%s! Error: ilLoadL() Error [%s]", error_string.c_str(), iluErrorString(ilGetError()));
		ret = false;
	}

	ilDeleteImages(1, &il_image);

	return ret;
}

uint Importer::Textures::Utilities::CreateTexture(const void* data, uint width, uint height, uint target, int wrapping, int filter, int internal_format, uint format)
{
	uint texture_id = 0; 

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, (GLuint*)&texture_id);
	glBindTexture(target, texture_id);

	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapping);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapping);

	if (filter == GL_NEAREST)																					// Nearest filtering gets the color of the nearest neighbour pixel.
	{
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	}
	else if (filter == GL_LINEAR)																				// Linear filtering interpolates the color of the neighbour pixels.
	{
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))												// In case Anisotropic filtering is available, it will be used.
		{
			GLfloat max_anisotropy;

			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
			glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)max_anisotropy);
		}
	}
	else
	{
		LOG("[ERROR] Invalid filter type! Supported filters: GL_LINEAR and GL_NEAREST.");
	}

	glTexImage2D(target, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(target);

	glBindTexture(target, 0);

	if (texture_id != 0)
	{
		LOG("[STATUS] Texture Successfully loaded! Id: %u, Size: %u x %u", texture_id, width, height);
	}

	return texture_id;
}