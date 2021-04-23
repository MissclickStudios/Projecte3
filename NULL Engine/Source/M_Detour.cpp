#include "M_Detour.h"

#include "Application.h"

#include "NavMeshManager.h"
#include "M_Scene.h"
#include "GameObject.h"
#include "C_Mesh.h"
#include "C_Transform.h"

M_Detour::M_Detour(bool isActive) : Module("Detour", isActive)
{
	//geom = new InputGeom();
}

M_Detour::~M_Detour()
{
}

bool M_Detour::Init(ParsonNode& root)
{
	return true;
}

bool M_Detour::Start()
{
	return true;
}

UpdateStatus M_Detour::Update(float dt)
{

	return UpdateStatus::CONTINUE;
}

bool M_Detour::CleanUp()
{
	return true;
}

void M_Detour::BakeNavMesh()
{

	/*std::vector<GameObject*>::const_iterator it = App->scene->GetGameObjects()->cbegin();

	for (it; it != App->scene->GetGameObjects()->cend(); ++it)
	{
		if ((*it) != nullptr && (*it)->IsStatic() == true)
			AddGameObjectNavMesh(*it);

	}*/
}

/*void M_Detour::AddGameObjectNavMesh(GameObject* gameObject)
{
	C_Mesh* componentMesh = gameObject->GetComponent<C_Mesh>();

	if (componentMesh != nullptr)
	{
		R_Mesh* rMesh = componentMesh->GetMesh();

		float4x4 mTransform = gameObject->GetComponent<C_Transform>()->GetWorldTransform();

		geom->CreateMesh(rMesh, mTransform);
		
		navMesh = new NavMeshManager();
		navMesh->handleMeshChanged(geom);
		navMesh->handleSettings();
		navMesh->handleBuild();
	}
}*/

void M_Detour::DrawNavMesh()
{
	//if(navMesh != nullptr)
		//navMesh->handleRender();
}
