#include "M_Detour.h"

#include "Application.h"

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

bool M_Detour::CleanUp()
{
	return true;
}

void M_Detour::Draw() const
{
}

void M_Detour::setDebugDraw(bool state)
{
}

bool M_Detour::createNavMesh(dtNavMeshCreateParams* params)
{
	return true;
}

void M_Detour::loadNavMeshFile(unsigned int UID)
{
}

void M_Detour::deleteNavMesh()
{
}

void M_Detour::clearNavMesh()
{
}

int M_Detour::getAreaCost(unsigned int areaIndex) const
{
	return 0;
}

void M_Detour::setAreaCost(unsigned int areaIndex, float areaCost)
{
}

int M_Detour::getAreaFromName(const char* name) const
{
	return 0;
}

int M_Detour::calculatePath(float3 sourcePosition, float3 destination, int areaMask, std::vector<float3>& path)
{
	return 0;
}

bool M_Detour::nearestPosInMesh(float3 sourcePosition, int areaMask, float3& nearestPoint)
{
	return false;
}

void M_Detour::setDefaultValues()
{
}

void M_Detour::setDefaultBakeValues()
{
}

const R_NavMesh* M_Detour::getNavMeshResource() const
{
	return nullptr;
}

void M_Detour::allocateNavMesh()
{
}

void M_Detour::createRenderMeshes()
{
}

void M_Detour::saveNavMesh() const
{
}

inline void M_Detour::initNavQuery()
{
}

bool M_Detour::createNavMeshData(dtNavMeshCreateParams* params, unsigned char** outData, int* outDataSize)
{
	return false;
}

unsigned int M_Detour::initNavMesh(const dtNavMeshParams* params)
{
	return 0;
}

void M_Detour::freeNavMeshData(void* ptr)
{
}

void M_Detour::setAreaCosts()
{
}

void M_Detour::processTile(const dtMeshTile* tile)
{
}

Color M_Detour::areaToColor(unsigned int area) const
{
	return Color();
}
