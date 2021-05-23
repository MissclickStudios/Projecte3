#ifndef __M_DETOUR_H__
#define __M_DETOUR_H__

#include "Module.h"
#include "Macros.h"
#include "Color.h"
#include "MathGeoLib/include/Math/float3.h"
#include "RecastNavigation/Detour/Include/DetourNavMesh.h"
#include "RecastNavigation/Detour/Include/DetourNavMeshQuery.h"

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
	UpdateStatus	Update(float dt) override;
	bool CleanUp() override;

	void Draw() const;
	void setDebugDraw(bool state);

	bool createNavMesh(dtNavMeshCreateParams* params);
	void loadNavMeshFile(unsigned int navMeshUid, const char* navMeshPath);

	void deleteNavMesh();
	void clearNavMesh();

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

	void LoadNavMeshBuffers(std::vector<navigationPoly*>& meshes);

public:
	// Scripting
	int getAreaCost(unsigned int areaIndex) const;
	void setAreaCost(unsigned int areaIndex, float areaCost);
	int getAreaFromName(const char* name) const;
	static int allAreas() { return POLYFLAGS_ALL; }
	// Will write the path to variable path, return number of verts
	bool CalculatePath(float3 sourcePosition, float3 destination, std::vector<float3>& path);
	bool nearestPosInMesh(float3 sourcePosition, int areaMask, float3& nearestPoint);

	void ReCalculatePath();

public:

	float agentRadius = 4.0f;
	float agentHeight = 2.0f;
	float agentMaxClimb = 0.9f;
	float maxSlopeAngle = 45.0f;
	float stepHeight = 0.4f;

	float m_cellSize = 0.3f;
	float m_cellHeight = 0.2f;

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

	std::vector<navigationPoly*> renderMeshes;
	bool debugDraw = true;
	R_NavMesh* navMeshResource = nullptr;

private:
	dtNavMeshQuery* m_navQuery = nullptr;
	dtQueryFilter* m_filterQuery = nullptr;
	dtStatus m_pathFindStatus;

	enum ToolMode
	{
		TOOLMODE_PATHFIND_FOLLOW,
		TOOLMODE_PATHFIND_STRAIGHT,
		TOOLMODE_PATHFIND_SLICED,
		TOOLMODE_RAYCAST,
		TOOLMODE_DISTANCE_TO_WALL,
		TOOLMODE_FIND_POLYS_IN_CIRCLE,
		TOOLMODE_FIND_POLYS_IN_SHAPE,
		TOOLMODE_FIND_LOCAL_NEIGHBOURHOOD,
	};

	//Pathfinding
	ToolMode m_toolMode;

	int m_straightPathOptions;

	static const int MAX_POLYS = 256;
	static const int MAX_SMOOTH = 2048;

	dtPolyRef m_startRef;
	dtPolyRef m_endRef;
	dtPolyRef m_polys[MAX_POLYS];
	dtPolyRef m_parent[MAX_POLYS];
	int m_npolys;
	float m_straightPath[MAX_POLYS * 3];
	unsigned char m_straightPathFlags[MAX_POLYS];
	dtPolyRef m_straightPathPolys[MAX_POLYS];
	int m_nstraightPath;
	float m_polyPickExt[3];
	float m_smoothPath[MAX_SMOOTH * 3];
	int m_nsmoothPath;
	float m_queryPoly[4 * 3];

	static const int MAX_RAND_POINTS = 64;
	float m_randPoints[MAX_RAND_POINTS * 3];
	int m_nrandPoints;
	bool m_randPointsInCircle;

	float3 m_startPos;
	float3 m_endPos;
	float m_hitPos[3];
	float m_hitNormal[3];
	float m_Extents[3];

	bool m_hitResult;
	float m_distanceToWall;
	float m_neighbourhoodRadius;
	float m_randomRadius;
	bool m_startPosSet;
	bool m_endPosSet;

	int m_pathIterNum;
	dtPolyRef m_pathIterPolys[MAX_POLYS];
	int m_pathIterPolyCount;
	float m_prevIterPos[3], m_iterPos[3], m_steerPos[3], m_targetPos[3];

	static const int MAX_STEER_POINTS = 10;
	float m_steerPoints[MAX_STEER_POINTS * 3];
	int m_steerPointCount;


};
#endif // __M_DETOUR_H__