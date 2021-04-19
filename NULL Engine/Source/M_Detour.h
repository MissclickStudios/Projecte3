#ifndef __M_DETOUR_H__
#define __M_DETOUR_H__

class NavMeshManager;
class InputGeom;
class GameObject;

#include "Module.h"

#include "Macros.h"

class NULL_API M_Detour : public Module
{
public:
	M_Detour(bool isActive = true);
	~M_Detour();

	bool			Init(ParsonNode& root) override;
	bool			Start() override;
	UpdateStatus	Update(float dt) override;
	bool			CleanUp() override;

	void BakeNavMesh();
	void AddGameObjectNavMesh(GameObject* gameObject);

	void DrawNavMesh();
private:

	InputGeom* geom = nullptr;
	NavMeshManager* navMesh = nullptr;

};
#endif // __M_DETOUR_H__