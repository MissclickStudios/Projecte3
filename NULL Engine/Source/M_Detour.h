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

#define BE_DETOUR_TOTAL_AREAS 31
#define BE_DETOUR_MAX_PATHSLOT 128 // how many paths we can store
#define BE_DETOUR_MAX_PATHPOLY 256 // max number of polygons in a path
#define BE_DETOUR_MAX_PATHVERT 512 // most verts in a path 

enum PolyFlags {
	POLYFLAGS_AREA_WALKABLE = 0x01,
	POLYFLAGS_AREA_JUMP = 0x02,
	POLYFLAGS_AREA_DISABLED = 0x04,
	POLYFLAGS_AREA_USER3 = 0x06,
	POLYFLAGS_AREA_USER4 = 0x08,
	POLYFLAGS_AREA_USER5 = 0x10,
	POLYFLAGS_AREA_USER6 = 0x20,
	POLYFLAGS_AREA_USER7 = 0x40,
	POLYFLAGS_AREA_USER8 = 0x60,
	POLYFLAGS_AREA_USER9 = 0x80,
	POLYFLAGS_AREA_USER10 = 0x100,
	POLYFLAGS_AREA_USER11 = 0x200,
	POLYFLAGS_AREA_USER12 = 0x400,
	POLYFLAGS_AREA_USER13 = 0x600,
	POLYFLAGS_AREA_USER14 = 0x800,
	POLYFLAGS_AREA_USER15 = 0x1000,
	POLYFLAGS_AREA_USER16 = 0x2000,
	POLYFLAGS_AREA_USER17 = 0x4000,
	POLYFLAGS_AREA_USER18 = 0x6000,
	POLYFLAGS_AREA_USER19 = 0x8000,
	POLYFLAGS_AREA_USER20 = 0x10000,
	POLYFLAGS_AREA_USER21 = 0x20000,
	POLYFLAGS_AREA_USER22 = 0x40000,
	POLYFLAGS_AREA_USER23 = 0x60000,
	POLYFLAGS_AREA_USER24 = 0x80000,
	POLYFLAGS_AREA_USER25 = 0x100000,
	POLYFLAGS_AREA_USER26 = 0x200000,
	POLYFLAGS_AREA_USER27 = 0x400000,
	POLYFLAGS_AREA_USER28 = 0x600000,
	POLYFLAGS_AREA_USER29 = 0x800000,
	POLYFLAGS_AREA_USER30 = 0x900000,
	POLYFLAGS_ALL = 0xffffff      // All abilities.
};

struct NULL_API navigationPoly {
	navigationPoly();
	~navigationPoly();

	R_Mesh* rmesh;
	Color color;
};

class NULL_API M_Detour : public Module
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
	void loadNavMeshFile(unsigned int UID);

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