#ifndef __COMPONENTPARTICLESYSTEM__
#define __COMPONENTPARTICLESYSTEM__

class EmitterInstance
{

};


class RParticleSystem;

class CParticleSystem : public Component
{
public:
	CParticleSystem(GameObject* owner);
	~CParticleSystem();

	void Update(float dt)override;
	void Reset();

	void Serialize(JsonNode* node)override;
	void Load(JsonNode* node)override;
	
private:

	RParticleSystem* rParticleSystem;

};
#endif //__COMPONENTPARTICLESYSTEM__