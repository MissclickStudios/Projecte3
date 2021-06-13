#ifndef __IMPORTER_H__
#define __IMPORTER_H__

class Resource;
class R_Model;
class R_Mesh;
class R_Texture;
class R_Shader;
class R_ParticleSystem;
class R_Script;
class R_NavMesh;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

namespace Importer
{
	void	InitializeImporters();
	void	ShutDownImporters();

	bool	ImportScene(const char* buffer, uint size, R_Model* rModel);
	bool	ImportMesh(const char* buffer, R_Mesh* rMesh);
	bool	ImportTexture(const char* buffer, uint size, R_Texture* rTexture);
	bool	ImportShader(const char* buffer, R_Shader* rShader);
	bool	ImportParticles(const char* buffer, R_ParticleSystem* rParticles);
	bool	ImportScript(const char* buffer, R_Script* rScript);
	bool	ImportNavMesh(const char* buffer, R_NavMesh* rNavMesh);

	void	FindAndForceUID(Resource* resource);											// MOVE TO RESOURCE MANAGER LATER.
}

#endif // !__IMPORTER_H__