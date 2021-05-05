#ifndef __M_DETOUR_H__
#define __M_DETOUR_H__

#include "Module.h"
#include "Macros.h"
#include "Color.h"
#include "MathGeoLib/include/Math/float3.h"

class InputGeom;
class GameObject;
class R_NavMesh;
class R_Mesh;
class R_Material;

struct dtNavMeshCreateParams;
struct dtNavMeshParams;
class dtNavMesh;
class dtNavMeshQuery;
class dtQueryFilter;
class dtMeshTile;

#define BE_DETOUR_TOTAL_AREAS 4
#define BE_DETOUR_MAX_PATHSLOT 128 // how many paths we can store
#define BE_DETOUR_MAX_PATHPOLY 256 // max number of polygons in a path
#define BE_DETOUR_MAX_PATHVERT 512 // most verts in a path 

enum PolyFlags {
	POLYFLAGS_AREA_WALKABLE = 0x01,
	POLYFLAGS_AREA_JUMP = 0x02,
	POLYFLAGS_AREA_DISABLED = 0x04,
	POLYFLAGS_ALL = 0xffffff      // All abilities.
};

struct MISSCLICK_API navigationPoly {
	navigationPoly();
	~navigationPoly();

	R_Mesh* rmesh;
	Color color;
};

class MISSCLICK_API M_Detour : public Module
{
	friend class M_Recast;
public:
	M_Detour(bool start_enabled = true);
	~M_Detour();

	bool Init(ParsonNode& config) override;
	bool CleanUp() override;

	void Draw() const;
	void setDebugDraw(bool state);

	bool createNavMesh(dtNavMeshCreateParams* params);
	void loadNavMeshFile(unsigned int navMeshUid, const char* navMeshPath);

	void deleteNavMesh();
	void clearNavMesh();

	// Functions for scripting
	int getAreaCost(unsigned int areaIndex) const;
	void setAreaCost(unsigned int areaIndex, float areaCost);
	int getAreaFromName(const char* name) const;
	static int allAreas() { return POLYFLAGS_ALL; }
	// Will write the path to variable path, return number of verts
	int calculatePath(float3 sourcePosition, float3 destination, int areaMask, std::vector<float3>& path);
	bool nearestPosInMesh(float3 sourcePosition, int areaMask, float3& nearestPoint);


	void setDefaultValues();
	void setDefaultBakeValues();
	const R_NavMesh* getNavMeshResource() const;
	void allocateNavMesh();
	void createRenderMeshes();
	void saveNavMesh() const;
	inline void initNavQuery();

	// For the sake of keeping memory in the dll heap
	bool createNavMeshData(dtNavMeshCreateParams* params, unsigned char** outData, int* outDataSize);
	unsigned int initNavMesh(const dtNavMeshParams* params);
	void freeNavMeshData(void* ptr);

private:
	void setAreaCosts();
	void processTile(const dtMeshTile* tile);

	//Copy of Detour dunsigned intToCol but without transforming to unsigned int
	Color areaToColor(unsigned int area) const;

public:
	float agentRadius = 0.5f;
	float agentHeight = 2.0f;
	float maxSlope = 45.0f;
	float stepHeight = 0.4f;
	float voxelSize = 0.15f;
	float voxelHeight = 0.2f;

	float regionMinSize = 8;
	float regionMergeSize = 20;
	float edgeMaxLen = 12.0f;
	float edgeMaxError = 1.3f;
	float vertsPerPoly = 6.0f;
	float detailSampleDist = 6.0f;
	float detailSampleMaxError = 1.0f;

	bool buildTiledMesh = true;

	char areaNames[BE_DETOUR_TOTAL_AREAS][100];
	float areaCosts[BE_DETOUR_TOTAL_AREAS];

private:
	bool debugDraw = false;
	R_NavMesh* navMeshResource = nullptr;
	dtNavMeshQuery* m_navQuery = nullptr;
	dtQueryFilter* m_filterQuery = nullptr;
	std::vector<navigationPoly*> renderMeshes;
	R_Material* mat = nullptr;
	float m_Extents[3];

};
#endif // __M_DETOUR_H__