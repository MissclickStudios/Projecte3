#include "R_Texture.h"
#include "R_Shader.h"

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
	return true;
}

bool RE_Particle::CleanUp()
{
	return true;
}