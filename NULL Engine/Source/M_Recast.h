#ifndef __M_Recast_H__
#define __M_Recast_H__
#pragma once


#include "Module.h"
#include "RecastNavigation/Recast/Include/Recast.h"
#include "MathGeoLib/include/Geometry/OBB.h"
#include <vector>

class InputGeom;
class ParsonNode;
class GameObject;


/// The maximum number of vertices per navigation polygon.
/// @ingroup detour
//static const int DT_VERTS_PER_POLYGON = 6;

class M_Recast : public Module {
public:
	M_Recast(bool isActive = true);
	~M_Recast();

	bool Init(ParsonNode& config) override;

	void AddGO(GameObject* go);
	void DeleteGO(GameObject* go);

	bool CleanUp() override;

	bool BuildNavMesh();
	bool BuildSoloNavMesh(const InputGeom* m_geom);
	bool BuildTiledNavMesh(const InputGeom* m_geom);
	unsigned char* BuildTile(const InputGeom* m_geom, const int tx, const int ty, const float* bmin, const float* bmax, int& datasize);

private:
	void MarkOBBArea(const math::OBB& obb, unsigned char areaId, rcCompactHeightfield& chf);

public:
	bool filterLowHangingObstacles = true;
	bool filterLedgeSpans = true;
	bool filterWalkableLowHeightSpans = true;

private:
	std::vector<GameObject*> NavigationGameObjects;

	rcContext* m_ctx = nullptr;

	//Recast variables
	unsigned char* m_triareas = nullptr;
	rcHeightfield* m_solid = nullptr;
	rcCompactHeightfield* m_chf = nullptr;
	rcContourSet* m_cset = nullptr;
	rcPolyMesh* m_pmesh = nullptr;
	rcPolyMeshDetail* m_dmesh = nullptr;
	rcConfig m_cfg;

	int m_maxTiles;
	int m_maxPolysPerTile;
	float m_tileSize;

	unsigned int m_tileCol;
	float m_lastBuiltTileBmin[3];
	float m_lastBuiltTileBmax[3];
	int m_tileTriCount;

};

#endif //__M_Recast_H__


