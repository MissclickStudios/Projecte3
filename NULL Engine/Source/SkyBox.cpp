#include "MathGeoLib/include/Math/float4x4.h"
#include "MathGeoLib/include/Math/float3x3.h"
#include "MathGeoLib/include/Math/TransformOps.h"

#include "OpenGL.h"
#include "DevIL.h"

#include "Macros.h"

#include "Application.h"
#include "M_Camera3D.h"
#include "M_ResourceManager.h"
#include "M_FileSystem.h"

#include "R_Shader.h"

#include "C_Camera.h"

#include "SkyBox.h"

#include "MemoryManager.h"

#pragma comment (lib, "Source/Dependencies/DevIL/libx86/DevIL.lib")
#pragma comment (lib, "Source/Dependencies/DevIL/libx86/ILU.lib")
#pragma comment (lib, "Source/Dependencies/DevIL/libx86/ILUT.lib")

#include <map>

Skybox::Skybox()
{
}

Skybox::~Skybox()
{
}

void Skybox::SetUpSkyBoxBuffers()
{	
	glGenBuffers(1, (GLuint*)&(skyboxId));
	glBindBuffer(GL_ARRAY_BUFFER, skyboxId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36 * 3, skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Skybox::CreateSkybox()
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		char* buffer			= nullptr;
		std::string textureFile = ASSETS_SKYBOX_PATH + faces[i];
		unsigned int size		= App->fileSystem->Load(textureFile.data(), &buffer);
		if (size == 0)
			continue;

		ILuint id;
		ilGenImages(1, &id);
		ilBindImage(id);
		if (ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size))
		{
			ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
			ILinfo info;
			iluGetImageInfo(&info);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, info.Width, info.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, info.Data);

			ilDeleteImages(1, &id);
		}
		
		RELEASE_ARRAY(buffer);
	}


	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	skyboxTexId = textureID;
}

void Skybox::RenderSkybox()
{

	/*glDepthMask(GL_FALSE);
	glCullFace(GL_FRONT);*/
	glDepthFunc(GL_LEQUAL);
	
	if (skyboxProgramId == 0)
	{
		skyboxProgramId = App->resourceManager->GetShader("SkyBoxShader")->shaderProgramID;
		glUseProgram(skyboxProgramId);
	}
	else if (skyboxProgramId != 0)
	{
		glUseProgram(skyboxProgramId);
	}
	else LOG("Error loading skybox shader program");


	float3 translation = App->camera->GetCurrentCamera()->GetFrustum().WorldMatrix().TranslatePart();
	float4x4 modelMatrix = math::float4x4::identity;
	modelMatrix.SetTranslatePart(translation);
	modelMatrix.Scale(50,50,50);

	math::float4x4 resultMatrix = modelMatrix.Transposed() * App->camera->currentCamera->GetViewMatrixTransposed() * App->camera->currentCamera->GetProjectionMatrixTransposed();

	GLint uinformLoc = glGetUniformLocation(skyboxProgramId, "resultMatrix");
	glUniformMatrix4fv(uinformLoc, 1, GL_FALSE, *resultMatrix.v);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexId);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDepthFunc(GL_LESS);
	glUseProgram(0);

}

void Skybox::CleanUp()
{
	//glDeleteVertexArrays(1, &skyboxId);
	glDeleteBuffers(1, &skyboxId);
	//glDeleteTextures(1, &skyboxTexId);
}
