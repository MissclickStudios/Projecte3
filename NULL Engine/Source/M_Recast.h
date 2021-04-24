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
	void BuildTile(const InputGeom* m_geom, const int tx, const int ty, unsigned char** data, int* datasize);

private:
	void MarkOBBArea(const math::OBB& obb, unsigned char areaId, rcCompactHeightfield& chf);

public:
	bool filterLowHangingObstacles = true;
	bool filterLedgeSpans = true;
	bool filterWalkableLowHeightSpans = true;

private:
	std::vector<GameObject*> NavigationGameObjects;

	//Recast variables
	rcConfig m_cfg;

};

#endif //__M_Recast_H__


