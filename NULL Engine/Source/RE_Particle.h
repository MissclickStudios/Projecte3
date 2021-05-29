#ifndef __RE_PARTICLE_H__
#define __RE_PARTICLE_H__

#include "Renderer.h"

class R_Texture;
class R_Shader;

typedef unsigned int uint;

const float Particles_Coords[] = {																// Change later to ParticlesCoords
	1,	1,
	1,	0,
	0,	0,
	1,	0,
};

class RE_Particle : public Renderer
{
public:
	RE_Particle(float4x4* transform, float lineWidth, Color color, R_Texture* rTexture, R_Shader* rShader);
	RE_Particle(float4x4* transform, R_Texture* rTexture, R_Shader* rShader);
	~RE_Particle();

	bool Render() override;
	bool CleanUp() override;

private:
	void LoadBuffers();

private:
	uint		VAO;

	R_Texture*	rTexture;
	R_Shader*	rShader;
};

#endif // !__RE_PARTICLE_H__