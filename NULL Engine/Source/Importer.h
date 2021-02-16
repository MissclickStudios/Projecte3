#ifndef __IMPORTER_H__
#define __IMPORTER_H__

class Resource;
class R_Model;
class R_Mesh;
class R_Texture;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

namespace Importer
{
	void	InitializeImporters	();
	void	ShutDownImporters	();

	bool	ImportScene			(const char* buffer, uint size, R_Model* r_model);
	bool	ImportMesh			(const char* buffer, R_Mesh* r_mesh);
	bool	ImportTexture		(const char* buffer, uint size, R_Texture* r_texture);
}

#endif // !__IMPORTER_H__