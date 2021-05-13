#ifndef __RE_PARTICLE_H__
#define __RE_PARTICLE_H__

#include "Renderer.h"

class R_Texture;
class R_Shader;

typedef unsigned int uint;

class RE_Particle : public Renderer
{
public:
	RE_Particle(float4x4* transform, float lineWidth, Color color, R_Texture* rTexture, R_Shader* rShader);
	RE_Particle(float4x4* transform, R_Texture* rTexture, R_Shader* rShader);
	~RE_Particle();

	bool Render() override;
	bool CleanUp() override;

private:

private:
	uint VAO;

	R_Texture*	rTexture;
	R_Shader*	rShader;
};

#endif // !__RE_PARTICLE_H__