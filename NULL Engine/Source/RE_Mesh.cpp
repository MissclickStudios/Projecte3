#include "MC_Time.h"

#include "Log.h"

#include "Application.h"
#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "R_Mesh.h"
#include "R_Shader.h"

#include "I_Shaders.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Material.h"
#include "C_Light.h"

#include "RE_Mesh.h"

RE_Mesh::RE_Mesh(C_Transform* cTransform, C_Mesh* cMesh, C_Material* cMaterial) : Renderer(RendererType::MESH, (cTransform != nullptr) ? cTransform->GetWorldTransformPtr() : nullptr),
cTransform		(cTransform),
cMesh			(cMesh),
cMaterial		(cMaterial),
boneTransforms	(nullptr)
{

}

RE_Mesh::~RE_Mesh()
{

}

bool RE_Mesh::Render()
{
	std::string name = cTransform->GetOwner()->GetName();

	R_Mesh* rMesh = cMesh->GetMesh();
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh! Error: R_Mesh* was nullptr.");
		return false;
	}

	glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glClear(GL_STENCIL_BUFFER_BIT);
	
	if (cMesh->GetOutlineMesh())
	{
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}
	else
	{
		glStencilMask(0x00);
	}

	ApplyDebugParameters();																									// Enable Wireframe Mode for this specific mesh, etc.
	ApplyTextureAndMaterial();																								// Apply resource texture or default texture, mesh color...
	ApplyShader();																											// 

	if (cMesh->GetSkinnedMesh() == nullptr)
	{
		glBindVertexArray(rMesh->VAO);
		glDrawElements(GL_TRIANGLES, rMesh->indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glBindVertexArray(cMesh->GetSkinnedMesh()->VAO);
		glDrawElements(GL_TRIANGLES, cMesh->GetSkinnedMesh()->indices.size(), GL_UNSIGNED_INT, nullptr);
	}

	if (cMesh->GetOutlineMesh())
	{
		RenderOutline(rMesh);
	}

	ClearShader();	

	glBindVertexArray(0);																									//

	glBindTexture(GL_TEXTURE_2D, 0);																						// ---------------------
	
	ClearTextureAndMaterial();																								// Clear the specifications applied in ApplyTextureAndMaterial().
	ClearDebugParameters();																									// Clear the specifications applied in ApplyDebugParameters().

	// --- DEBUG DRAW ---
	if (rMesh->drawVertexNormals || App->renderer->GetRenderVertexNormals())
	{
		RenderVertexNormals(rMesh);
	}
	if (rMesh->drawFaceNormals || App->renderer->GetRenderFaceNormals())
	{
		RenderFaceNormals(rMesh);
	}
	
	return true;
}

bool RE_Mesh::CleanUp()
{
	return true;
}

// --- RE_MESH METHODS ---
void RE_Mesh::RenderVertexNormals(const R_Mesh* rMesh)
{
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Vertex Normals! Error: R_Mesh* was nullptr.");
		return;
	}
	
	if (rMesh->vertices.size() != rMesh->normals.size())
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Vertex Normals! Error: Num Vertices != Num Normals.");
		return;
	}

	std::vector<float> vertices	= rMesh->vertices;
	std::vector<float> normals	= rMesh->normals;

	glColor4fv(App->renderer->GetVertexNormalsColor().C_Array());
	glLineWidth(App->renderer->GetVertexNormalsWidth());
	glBegin(GL_LINES);

	for (uint i = 0; i < vertices.size(); i += 3)
	{
		glVertex3f(vertices[i], vertices[i + 1], vertices[i + 2]);
		glVertex3f(vertices[i] + normals[i], vertices[i + 1] + normals[i + 1], vertices[i + 2] + normals[i + 2]);
	}

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Clear vectors?
}

void RE_Mesh::RenderFaceNormals(const R_Mesh* rMesh)
{
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Face Normals! Error: R_Mesh* was nullptr.");
		return;
	}
	
	if (rMesh->vertices.size() != rMesh->normals.size())
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Face Normals! Error: Num Vertices != Num Normals.");
		return;
	}

	std::vector<Triangle> vertexFaces;
	std::vector<Triangle> normalFaces;
	GetFaces(rMesh, vertexFaces, normalFaces);

	if (vertexFaces.size() != normalFaces.size())
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Face Normals! Error: Num Vertex Faces != Num Normal Faces.");
		return;
	}

	glColor4fv(App->renderer->GetFaceNormalsColor().C_Array());
	glLineWidth(App->renderer->GetFaceNormalsWidth());
	glBegin(GL_LINES);


	for (uint i = 0; i < vertexFaces.size(); ++i)
	{
		float3 origin		= vertexFaces[i].CenterPoint();												// Center vector of Face Vertices
		float3 destination	= origin + normalFaces[i].CenterPoint();									// Center of Face Vertices + Center of Face Normals.

		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(destination.x, destination.y, destination.z);
	}

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	vertexFaces.clear();
	normalFaces.clear();
}

void RE_Mesh::GetFaces(const R_Mesh* rMesh, std::vector<Triangle>& vertexFaces, std::vector<Triangle>& normalFaces)
{
	std::vector<float3> vertices;
	std::vector<float3> normals;
	
	for (uint i = 0; i < rMesh->vertices.size(); i += 3)
	{
		float3 vertex;
		float3 normal;

		vertex.x = rMesh->vertices[i];
		vertex.y = rMesh->vertices[i + 1];
		vertex.z = rMesh->vertices[i + 2];

		normal.x = rMesh->normals[i];
		normal.y = rMesh->normals[i + 1];
		normal.z = rMesh->normals[i + 2];

		vertices.push_back(vertex);
		normals.push_back(normal);

		if (vertices.size() == 3 && normals.size() == 3)
		{
			vertexFaces.push_back(Triangle(vertices[0], vertices[1], vertices[2]));
			normalFaces.push_back(Triangle(normals[0], normals[1], normals[2]));

			vertices.clear();
			normals.clear();
		}
	}

	vertices.clear();
	normals.clear();
}

void RE_Mesh::ApplyDebugParameters()
{
	if (App->renderer->GetRenderWireframes() || cMesh->GetShowWireframe())
	{
		glColor4fv(App->renderer->GetWireframeColor().C_Array());
		glLineWidth(App->renderer->GetWireframeLineWidth());
	}

	if (cMesh->GetShowWireframe() && !App->renderer->GetRenderWireframes())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		//glDisable(GL_LIGHTING);
	}
}

void RE_Mesh::ClearDebugParameters()
{
	if (cMesh->GetShowWireframe() && !App->renderer->GetRenderWireframes())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}

	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RE_Mesh::ApplyTextureAndMaterial()
{
	if (cMaterial != nullptr)
	{
		if (!cMaterial->IsActive())
		{
			glDisable(GL_TEXTURE_2D);
		}

		if (cMaterial->GetTexture() == nullptr)														// If the Material Component does not have a Texture Resource.
		{
			Color color = cMaterial->GetMaterialColour();
			glColor4f(color.r, color.g, color.b, color.a);											// Apply the diffuse color to the mesh.
		}
		else if (cMaterial->UseDefaultTexture())													// If the default texture is set to be used (bool use_default_texture)
		{
			glBindTexture(GL_TEXTURE_2D, App->renderer->GetDebugTextureID());						// Binding the texture that will be rendered. Index = 0 means we are clearing the binding.
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, cMaterial->GetTextureID());								// Binding the texture_id in the Texture Resource of the Material Component.
		}
	}
}

void RE_Mesh::ClearTextureAndMaterial()
{
	if (cMaterial != nullptr)
	{
		if (!cMaterial->IsActive())
		{
			glEnable(GL_TEXTURE_2D);
		}
	}
}

void RE_Mesh::RenderOutline(R_Mesh* rMesh)
{
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);

	if (cMaterial != nullptr)
	{
		R_Shader* tempShader = App->resourceManager->GetShader("OutlineShader");
		uint32 shaderProgram = (tempShader != nullptr) ? tempShader->shaderProgramID : 0;															// THIS
		if (shaderProgram != 0)
		{
			glUseProgram(shaderProgram);																											// THIS

			tempShader->SetUniform1f("outlineThickness",		cMesh->GetOutlineThickness());
			tempShader->SetUniformVec4f("outlineColor",			(GLfloat*)&cMesh->GetOutlineColor());
			tempShader->SetUniformMatrix4("modelMatrix",		cTransform->GetWorldTransform().Transposed().ptr());
			tempShader->SetUniformMatrix4("viewMatrix",			App->camera->GetCurrentCamera()->GetViewMatrixTransposed().ptr());
			tempShader->SetUniformMatrix4("projectionMatrix",	App->camera->GetCurrentCamera()->GetProjectionMatrixTransposed().ptr());

			//Animations
			bool hasSkinnedMesh = (cMesh->GetSkinnedMesh() != nullptr);

			tempShader->SetUniform1i("activeAnimation", hasSkinnedMesh);																			// THIS

			if (hasSkinnedMesh)
			{
				boneTransforms = cMesh->GetBoneTransformsAsPtr();
				
				if ((boneTransforms != nullptr) && !boneTransforms->empty())
				{
					tempShader->SetUniformMatrix4("finalBonesMatrices", (GLfloat*)(boneTransforms->begin()->ptr()), boneTransforms->size());
				}
			}
		}
	}

	if (cMesh->GetSkinnedMesh() == nullptr)
	{
		glBindVertexArray(rMesh->VAO);
		glDrawElements(GL_TRIANGLES, rMesh->indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glBindVertexArray(cMesh->GetSkinnedMesh()->VAO);
		glDrawElements(GL_TRIANGLES, cMesh->GetSkinnedMesh()->indices.size(), GL_UNSIGNED_INT, nullptr);
	}

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}

void RE_Mesh::ApplyShader()
{
	if (cMaterial == nullptr)
		return;

	std::vector<GameObject*> dirLights;																					// This is done each frame per Renderer.
	std::vector<GameObject*> pointLights;																				// Pass to ptr?
	App->scene->GetDirLights(dirLights);																				// Leave as is?
	App->scene->GetPointLights(pointLights);																			// Either way maybe this is not a chokepoint at all.

	R_Shader* rShader = cMaterial->GetShader();																			// THIS. Having a local R_Shader* will save a lot of unnecessary calls.
	if (rShader == nullptr)
	{
		SetDefaultShader(cMaterial);																					// THIS. Defaulting to Default Shader in case there is none assigned.
		rShader = cMaterial->GetShader();
	}

	uint32 shaderProgram = rShader->shaderProgramID;
	if (shaderProgram == 0)
		return;

	glUseProgram(shaderProgram);

	(cMaterial->GetTexture() != nullptr)	? rShader->SetUniform1i("hasTexture", (GLint)true)	: rShader->SetUniform1i("hasTexture", (GLint)false);
	(cMaterial->GetTakeDamage())			? rShader->SetUniform1i("takeDamage", (GLint)true)	: rShader->SetUniform1i("takeDamage", (GLint)false);
	(!dirLights.empty())					? rShader->SetUniform1i("useDirLight", (GLint)true)	: rShader->SetUniform1i("useDirLight", (GLint)false);

	//Model
	rShader->SetUniformVec4f	("inColor",				(GLfloat*)&cMaterial->GetMaterialColour());
	rShader->SetUniformVec4f	("alternateColor",		(GLfloat*)&cMaterial->GetAlternateColour());
	rShader->SetUniformMatrix4	("modelMatrix",			cTransform->GetWorldTransform().Transposed().ptr());
	rShader->SetUniformMatrix4	("viewMatrix",			App->camera->GetCurrentCamera()->GetViewMatrixTransposed().ptr());
	rShader->SetUniformMatrix4	("projectionMatrix",	App->camera->GetCurrentCamera()->GetProjectionMatrixTransposed().ptr());
	rShader->SetUniformVec3f	("cameraPosition",		(GLfloat*)&App->camera->GetCurrentCamera()->GetFrustum().Pos());
	
	rShader->SetUniform1f		("deltaTime",			MC_Time::Game::GetDT());
	rShader->SetUniform1f		("Time",				MC_Time::Game::GetTimeSinceStart());

 	//Skybox
	rShader->SetUniform1i("skybox", 11);

	//Animations
	bool hasActiveAnimation = (cMesh->GetSkinnedMesh() != nullptr);																		// THIS. Changed the boolean's name.
	
	rShader->SetUniform1i("activeAnimation", hasActiveAnimation);																		

	if (hasActiveAnimation)																												// THIS. Now the ops. are done only when needed.
	{
		boneTransforms = cMesh->GetBoneTransformsAsPtr();																				// THIS. Get a Ptr instead of the whole vector.

		if ((boneTransforms != nullptr) && !boneTransforms->empty())																	// THIS. Checking nullptr.
		{	
			rShader->SetUniformMatrix4("finalBonesMatrices", (GLfloat*)(boneTransforms->begin()->ptr()), boneTransforms->size());
		}
	}

	// Light 
	if (!dirLights.empty())
	{
		for (uint i = 0; i < dirLights.size(); i++)
		{
			DirectionalLight* dirLight = dirLights[i]->GetComponent<C_Light>()->GetDirectionalLight();
			
			rShader->SetUniformVec4f("dirLight.diffuse",	(GLfloat*)&dirLight->diffuse);
			rShader->SetUniformVec4f("dirLight.ambient",	(GLfloat*)&dirLight->ambient);
			rShader->SetUniformVec4f("dirLight.specular",	(GLfloat*)&dirLight->specular);
			rShader->SetUniformVec3f("dirLight.direction",	(GLfloat*)&dirLight->GetDirection());

			rShader->SetUniformVec3f("viewPos",	(GLfloat*)&App->camera->GetCurrentCamera()->GetFrustum().Pos());
		}
	}

	if (!pointLights.empty())
	{
		for (uint i = 0; i < pointLights.size(); i++)
		{
			PointLight* pointLight = pointLights[i]->GetComponent<C_Light>()->GetPointLight();
			
			rShader->SetUniform1i		("numPointLights", pointLights.size());
			rShader->SetUniformVec4f	("pointLight[" + std::to_string(i) + "].diffuse",		(GLfloat*)&pointLight->diffuse);
			rShader->SetUniformVec4f	("pointLight[" + std::to_string(i) + "].ambient",		(GLfloat*)&pointLight->ambient);
			rShader->SetUniformVec4f	("pointLight[" + std::to_string(i) + "].specular",		(GLfloat*)&pointLight->specular);
			rShader->SetUniform1f		("pointLight[" + std::to_string(i) + "].constant",		pointLight->GetConstant());
			rShader->SetUniform1f		("pointLight[" + std::to_string(i) + "].linear",		pointLight->GetLinear());
			rShader->SetUniform1f		("pointLight[" + std::to_string(i) + "].quadratic",		pointLight->GetQuadratic());
			rShader->SetUniformVec3f	("pointLight[" + std::to_string(i) + "].position",		(GLfloat*)&pointLights[i]->transform->GetWorldPosition());
		}
	}

	Importer::Shaders::SetShaderUniforms(rShader);

	dirLights.clear();
	pointLights.clear();
}

uint32 RE_Mesh::SetDefaultShader(C_Material* cMaterial)
{
	//Assign the default Shader
	if (!App->renderer->defaultShader)
	{
		App->renderer->defaultShader = App->resourceManager->GetShader("DefaultShader");
	}

	cMaterial->SetShader(App->renderer->defaultShader);

	return cMaterial->GetShaderProgramID();
}

void RE_Mesh::ClearShader()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glUseProgram(0);
}
