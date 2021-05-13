#include "Application.h"
#include "M_Camera3D.h"

#include "R_Texture.h"
#include "R_Shader.h"

#include "C_Camera.h"

#include "RE_Particle.h"

RE_Particle::RE_Particle(float4x4* transform, float lineWidth, Color color, R_Texture* rTexture, R_Shader* rShader) : Renderer(RendererType::PARTICLE, transform, lineWidth, color),
rTexture	(rTexture),
rShader		(rShader),
VAO			(0)
{

}

RE_Particle::RE_Particle(float4x4* transform, R_Texture* rTexture, R_Shader* rShader) : Renderer(RendererType::PARTICLE, transform),
rTexture	(rTexture),
rShader		(rShader),
VAO			(0)
{

}

RE_Particle::~RE_Particle()
{

}

bool RE_Particle::Render()
{
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	glUseProgram(rShader->shaderProgramID);

	glBindTexture(GL_TEXTURE_2D, rTexture->GetTextureID());

	rShader->SetUniformMatrix4	("modelMatrix",			(GLfloat*)transform);
	rShader->SetUniformMatrix4	("viewMatrix",			App->camera->GetCurrentCamera()->GetViewMatrixTransposed().ptr());
	rShader->SetUniformMatrix4	("projectionMatrix",	App->camera->GetCurrentCamera()->GetProjectionMatrixTransposed().ptr());
	rShader->SetUniformVec4f	("color",				(GLfloat*)&color);

	glBindBuffer(GL_ARRAY_BUFFER, VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	
	return true;
}

bool RE_Particle::CleanUp()
{
	return true;
}

// --- RE_PARTICLE METHODS ---
void RE_Particle::LoadBuffers()
{
	glGenBuffers(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(ParticlesCoords), ParticlesCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
