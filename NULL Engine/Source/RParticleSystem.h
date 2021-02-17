#ifndef __RPARTICLESYSTEM_H__
#define __RPARTICLESYSTEM_H__

class Resource;

class ParticleModule
{

};

class Emitter
{

public:
	std::string name;
	std::vector<ParticleModule*> modules;
};


class RParticleSystem : Resource
{
public:
	RParticleSystem();
	RParticleSystem(uint UID);
	~RParticleSystem();
	void CleanUp() override;

private:

	//std::vector<Emitter>;
};

#endif //__RPARTICLESYSTEM_H__