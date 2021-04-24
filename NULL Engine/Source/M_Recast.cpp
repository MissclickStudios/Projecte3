#include "M_Recast.h"

#include "RecastNavigation/Detour/Include/DetourNavMeshBuilder.h"
#include "RecastNavigation/Detour/Include/DetourNavMesh.h"
#include "RecastNavigation/DebugUtils/Include/RecastDebugDraw.h"
#include "R_NavMesh.h"

#include "EngineApplication.h"		

#include "M_Renderer3D.h"
#include "M_Detour.h"

#include "R_Material.h"

#include "GameObject.h"

#include "Log.h"
#include "RecastNavigation/InputGeom.h"
#include "mmgr/include/mmgr.h"


inline unsigned int nextPow2(unsigned int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline unsigned int ilog2(unsigned int v) {
	unsigned int r;
	unsigned int shift;
	r = (v > 0xffff) << 4; v >>= r;
	shift = (v > 0xff) << 3; v >>= shift; r |= shift;
	shift = (v > 0xf) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}

M_Recast::M_Recast(bool isActive) : Module("Recast", isActive)
{
}

M_Recast::~M_Recast()
{
}

bool M_Recast::Init(ParsonNode& config)
{
	return true;
}

void M_Recast::AddGO(GameObject* go)
{
	if (go->isNavigable == true)
		NavigationGameObjects.push_back(go);
}

void M_Recast::DeleteGO(GameObject* go)
{
	for (std::vector<GameObject*>::iterator it = NavigationGameObjects.begin(); it != NavigationGameObjects.end(); ++it) {
		if (*it == go) {
			NavigationGameObjects.erase(it);
			break;
		}
	}
}

bool M_Recast::CleanUp()
{
	return true;
}

bool M_Recast::BuildNavMesh()
{
	bool ret = false;
	LOG("[Recast mesh]: Starting Nav Mesh build");

	if (NavigationGameObjects.size() == 0) {
		LOG("[Recast mesh]: No input mesh");
		ret = false;
	}

	InputGeom* m_geom = new InputGeom(NavigationGameObjects, EngineApp->detour->buildTiledMesh);

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = EngineApp->detour->voxelSize;
	m_cfg.ch = EngineApp->detour->voxelHeight;
	m_cfg.walkableSlopeAngle = EngineApp->detour->maxSlope;
	m_cfg.walkableHeight = (int)ceilf(EngineApp->detour->agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(EngineApp->detour->stepHeight / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(EngineApp->detour->agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(EngineApp->detour->edgeMaxLen / m_cfg.cs);
	m_cfg.maxSimplificationError = EngineApp->detour->edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(EngineApp->detour->regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(EngineApp->detour->regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)EngineApp->detour->vertsPerPoly;
	m_cfg.detailSampleDist = EngineApp->detour->detailSampleDist < 0.9f ? 0 : m_cfg.cs * EngineApp->detour->detailSampleDist;
	m_cfg.detailSampleMaxError = m_cfg.cs * EngineApp->detour->detailSampleMaxError;
	rcVcopy(m_cfg.bmin, m_geom->getMeshBoundsMin());
	rcVcopy(m_cfg.bmax, m_geom->getMeshBoundsMax());

	if (EngineApp->detour->buildTiledMesh)
		ret = BuildTiledNavMesh(m_geom);
	else
		ret = BuildSoloNavMesh(m_geom);

	delete m_geom;


	return ret;
}

bool M_Recast::BuildSoloNavMesh(const InputGeom* m_geom)
{//
		// Step 1. Initialize build config.
		//
		// Reset build times gathering.
	rcContext* m_ctx = new rcContext();
	m_ctx->resetTimers();

	// Start the build process.
	m_ctx->startTimer(RC_TIMER_TOTAL);

	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	int nverts = m_geom->getVertCount();
	int ntris = m_geom->getTriCount();

	LOG("[Recast mesh]: Building navigation:");
	LOG("[Recast mesh]: - %d x %d cells", m_cfg.width, m_cfg.height);
	LOG("[Recast mesh]: - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);


	//
	// Step 2. Rasterize input polygon soup.
	//

	// Allocate voxel heightfield where we rasterize our input data to.
	rcHeightfield* m_solid = rcAllocHeightfield();
	if (!m_solid) {
		LOG("[Recast mesh]: Out of memory 'solid'.");
		return false;
	}
	if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch)) {
		LOG("[Recast mesh]: Could not create solid heightfield.");
		return false;
	}

	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	unsigned char* m_triareas = new unsigned char[m_geom->getMaxTris()];
	if (!m_triareas) {
		LOG("[Recast mesh]: Out of memory 'm_triareas' (%d).", m_geom->getMaxTris());
		return false;
	}

	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	for (int i = 0; i

		< m_geom->getMeshes().size(); ++i) {
		memset(m_triareas, 0, m_geom->getMaxTris() * sizeof(unsigned char));
		// Modified recast method to set the area of the mesh directly instead of having to mark it later
		rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, m_geom->getVerts(), m_geom->getMeshes()[i].nverts, m_geom->getMeshes()[i].tris, m_geom->getMeshes()[i].ntris, m_triareas, m_geom->getMeshes()[i].area);
		rcRasterizeTriangles(m_ctx, m_geom->getVerts(), m_geom->getMeshes()[i].nverts, m_geom->getMeshes()[i].tris, m_triareas, m_geom->getMeshes()[i].ntris, *m_solid, m_cfg.walkableClimb);
	}

	delete[] m_triareas;
	m_triareas = nullptr;

	//
	// Step 3. Filter walkables surfaces.
	//

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
	if (filterLedgeSpans)
		rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	if (filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield* m_chf = rcAllocCompactHeightfield();
	if (!m_chf) {
		LOG("[Recast mesh]: Out of memory 'chf'.");
		return false;
	}
	if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf)) {
		LOG("[Recast mesh]: Could not build compact data.");
		return false;
	}

	rcFreeHeightField(m_solid);
	m_solid = nullptr;

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf)) {
		LOG("[Recast mesh]: Could not erode.");
		return false;
	}

	// (Optional) Mark areas.
	const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
		MarkOBBArea(vols[i].obb, vols[i].area, *m_chf);
	/*rcMarkConvexPolyArea(m_ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, vols[i].area, *m_chf);*/


 // Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
 // Using Watershed partitioning
 // Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(m_ctx, *m_chf)) {
		LOG("[Recast mesh]: Could not build distance field.");
		return false;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea)) {
		LOG("[Recast mesh]: Could not build regions.");
		return false;
	}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	rcContourSet* m_cset = rcAllocContourSet();
	if (!m_cset) {
		LOG("[Recast mesh]: Out of memory 'cset'.");
		return false;
	}
	if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset)) {
		LOG("[Recast mesh]: Could not create contours.");
		return false;
	}

	//
	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	rcPolyMesh* m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh) {
		LOG("[Recast mesh]: Out of memory 'pmesh'.");
		return false;
	}
	if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh)) {
		LOG("[Recast mesh]: Could not triangulate contours.");
		return false;
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	rcPolyMeshDetail* m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh) {
		LOG("[Recast mesh]: Out of memory 'pmdtl'.");
		return false;
	}

	if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh)) {
		LOG("[Recast mesh]: Could not build detail mesh.");
		return false;
	}

	rcFreeCompactHeightfield(m_chf);
	m_chf = nullptr;
	rcFreeContourSet(m_cset);
	m_cset = nullptr;


	// At this point the navigation mesh data is ready, you can access it from m_pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//

	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON) {
		unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		for (int i = 0; i < m_pmesh->npolys; ++i) {
			if (m_pmesh->areas[i] == RC_WALKABLE_AREA) {
				m_pmesh->areas[i] = 0; // Id of the walkable area
			}
			m_pmesh->flags[i] = 1 << m_pmesh->areas[i];
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;

		//No offmesh connections yet
		params.offMeshConCount = m_geom->getOffMeshConnectionCount();
		params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		params.offMeshConUserID = m_geom->getOffMeshConnectionId();


		params.walkableHeight = EngineApp->detour->agentHeight;
		params.walkableRadius = EngineApp->detour->agentRadius;
		params.walkableClimb = EngineApp->detour->stepHeight;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		EngineApp->detour->createNavMesh(&params);
	}

	if (m_pmesh)
		rcFreePolyMesh(m_pmesh);
	m_pmesh = nullptr;

	if (m_dmesh)
		rcFreePolyMeshDetail(m_dmesh);
	m_dmesh = nullptr;

	return true;
}

bool M_Recast::BuildTiledNavMesh(const InputGeom* m_geom)
{
	EngineApp->detour->allocateNavMesh();
	dtNavMesh* m_navmesh = EngineApp->detour->navMeshResource->navMesh;

	if (!m_navmesh) {
		LOG("[Recast mesh]: Could not allocate navmesh");
		return false;
	}

	// --- Tiled specific configurations ---
	m_cfg.tileSize = (int)(250 / m_cfg.cs);
	m_cfg.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
	m_cfg.width = m_cfg.tileSize + m_cfg.borderSize * 2;
	m_cfg.height = m_cfg.tileSize + m_cfg.borderSize * 2;

	int gw = 0, gh = 0;
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &gw, &gh);
	const int ts = (int)m_cfg.tileSize;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;

	int tileBits = math::Min<int>((int)ilog2(nextPow2(tw * th)), 14);
	if (tileBits > 14) tileBits = 14;
	int polyBits = 22 - tileBits;

	dtNavMeshParams params;
	rcVcopy(params.orig, m_geom->getMeshBoundsMin());
	params.tileWidth = ts * m_cfg.cs;
	params.tileHeight = ts * m_cfg.cs;
	params.maxTiles = 1 << tileBits;;
	params.maxPolys = 1 << polyBits;;

	dtStatus status;

	status = EngineApp->detour->initNavMesh(&params);
	if (dtStatusFailed(status)) {
		LOG("[Recast mesh]: Could not init navmesh.");
		return false;
	}

	const int totalTiles = th * tw;
	std::vector<unsigned char*> data;
	std::vector<int> dataSize;
	data.resize(totalTiles);
	dataSize.resize(totalTiles);

	for (int y = 0; y < th; ++y) {
		for (int x = 0; x < tw; ++x) {
			BuildTile(m_geom, x, y, &data[y * tw + x], &dataSize[y * tw + x]);
		}
	}

	for (int y = 0; y < th; ++y) {
		for (int x = 0; x < tw; ++x) {
			if (data[y * tw + x]) {
				//m_navmesh->removeTile(m_navmesh->getTileRefAt(x, y, 0), 0, 0);
				// Let the navmesh own the data.
				dtStatus status = m_navmesh->addTile(data[y * tw + x], dataSize[y * tw + x], DT_TILE_FREE_DATA, 0, 0);
				if (dtStatusFailed(status))
					EngineApp->detour->freeNavMeshData(data[y * tw + x]);
			}
		}
	}

	// We save the navmesh, create its render meshes and initialize the navQuery
	EngineApp->detour->saveNavMesh();
	EngineApp->detour->initNavQuery();
	EngineApp->detour->createRenderMeshes();

	return true;
}

void M_Recast::BuildTile(const InputGeom* m_geom, const int tx, const int ty, unsigned char** data, int* datasize)
{
	const float* verts = m_geom->getVerts();
	const int nverts = m_geom->getVertCount();
	const int ntris = m_geom->getTriCount();
	const rcChunkyTriMesh* chunkyMesh = m_geom->getChunkyMesh();

	rcConfig tile_cfg;
	rcContext tile_ctx;
	memcpy(&tile_cfg, &m_cfg, sizeof(rcConfig));

	const float tcs = tile_cfg.tileSize * tile_cfg.cs;
	tile_cfg.bmin[0] = m_cfg.bmin[0] + tx * tcs;
	tile_cfg.bmin[1] = m_cfg.bmin[1];
	tile_cfg.bmin[2] = m_cfg.bmin[2] + ty * tcs;

	tile_cfg.bmax[0] = m_cfg.bmin[0] + (tx + 1) * tcs;
	tile_cfg.bmax[1] = m_cfg.bmax[1];
	tile_cfg.bmax[2] = m_cfg.bmin[2] + (ty + 1) * tcs;


	// Expand the heighfield bounding box by border size to find the extents of geometry we need to build this tile.
	//
	// This is done in order to make sure that the navmesh tiles connect correctly at the borders,
	// and the obstacles close to the border work correctly with the dilation process.
	// No polygons (or contours) will be created on the border area.
	//
	// IMPORTANT!
	//
	//   :''''''''':
	//   : +-----+ :
	//   : |     | :
	//   : |     |<--- tile to build
	//   : |     | :  
	//   : +-----+ :<-- geometry needed
	//   :.........:
	//
	// You should use this bounding box to query your input geometry.
	//
	// For example if you build a navmesh for terrain, and want the navmesh tiles to match the terrain tile size
	// you will need to pass in data from neighbour terrain tiles too! In a simple case, just pass in all the 8 neighbours,
	// or use the bounding box below to only pass in a sliver of each of the 8 neighbours.

	tile_cfg.bmin[0] -= tile_cfg.borderSize * tile_cfg.cs;
	tile_cfg.bmin[2] -= tile_cfg.borderSize * tile_cfg.cs;
	tile_cfg.bmax[0] += tile_cfg.borderSize * tile_cfg.cs;
	tile_cfg.bmax[2] += tile_cfg.borderSize * tile_cfg.cs;

	// Reset build times gathering.
	tile_ctx.resetTimers();

	// Start the build process.
	tile_ctx.startTimer(RC_TIMER_TOTAL);

	// Allocate voxel heightfield where we rasterize our input data to.
	rcHeightfield* m_solid = rcAllocHeightfield();
	if (!m_solid) {
		LOG("[Recast tile]: Failed to allocate solid heightfield.");
		return;
	}
	if (!rcCreateHeightfield(&tile_ctx, *m_solid, tile_cfg.width, tile_cfg.height, tile_cfg.bmin, tile_cfg.bmax, tile_cfg.cs, tile_cfg.ch)) {
		LOG("[Recast tile]: Could not create solid heightfield.");
		return;
	}

	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	uchar* m_triareas = new uchar[chunkyMesh->maxTrisPerChunk];
	if (!m_triareas) {
		LOG("[Recast tile]: Out of memory 'm_triareas' (%d).", chunkyMesh->maxTrisPerChunk);
		return;
	}

	float tbmin[2], tbmax[2];
	tbmin[0] = m_cfg.bmin[0];
	tbmin[1] = m_cfg.bmin[2];
	tbmax[0] = m_cfg.bmax[0];
	tbmax[1] = m_cfg.bmax[2];
	int cid[512];// TODO: Make grow when returning too many items.
	const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
	if (!ncid)
		return;

	int m_tileTriCount = 0;

	for (int i = 0; i < ncid; ++i) {
		const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
		const int* ctris = &chunkyMesh->tris[node.i * 3];
		const uchar* careas = &chunkyMesh->areas[node.i];
		const int nctris = node.n;

		m_tileTriCount += nctris;

		memset(m_triareas, 0, nctris * sizeof(unsigned char));
		rcMarkWalkableTriangles(&tile_ctx, tile_cfg.walkableSlopeAngle,
			verts, nverts, ctris, careas, nctris, m_triareas);

		if (!rcRasterizeTriangles(&tile_ctx, verts, nverts, ctris, m_triareas, nctris, *m_solid, tile_cfg.walkableClimb)) {
			LOG("[Recast tile]: Could not rasterize triangles of tile x: %d, y: %d", tx, ty);
			return;
		}
	}

	delete[] m_triareas;
	m_triareas = 0;

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(&tile_ctx, tile_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(&tile_ctx, tile_cfg.walkableHeight, tile_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(&tile_ctx, tile_cfg.walkableHeight, *m_solid);

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield* m_chf = rcAllocCompactHeightfield();
	if (!m_chf) {
		LOG("[Recast tile]:  Out of memory 'chf'.");
		return;
	}
	if (!rcBuildCompactHeightfield(&tile_ctx, tile_cfg.walkableHeight, tile_cfg.walkableClimb, *m_solid, *m_chf)) {
		LOG("[Recast tile]: Could not build compact data.");
		return;
	}

	rcFreeHeightField(m_solid);
	m_solid = 0;

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(&tile_ctx, tile_cfg.walkableRadius, *m_chf)) {
		LOG("[Recast tile]: Could not erode.");
		return;
	}

	// (Optional) Mark areas.
	const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
		MarkOBBArea(vols[i].obb, vols[i].area, *m_chf);


	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 martitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
	// 2) Monotone partioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles

	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(&tile_ctx, *m_chf)) {
		LOG("[Recast tile]: Could not build distance field.");
		return;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(&tile_ctx, *m_chf, tile_cfg.borderSize, tile_cfg.minRegionArea, tile_cfg.mergeRegionArea)) {
		LOG("[Recast tile]: Could not build watershed regions.");
		return;
	}

	// Create contours.
	rcContourSet* m_cset = rcAllocContourSet();
	if (!m_cset) {
		LOG("[Recast tile]: Out of memory 'cset'.");
		return;
	}
	if (!rcBuildContours(&tile_ctx, *m_chf, tile_cfg.maxSimplificationError, tile_cfg.maxEdgeLen, *m_cset)) {
		LOG("[Recast tile]: Could not create contours.");
		return;
	}

	if (m_cset->nconts == 0) {
		return;
	}

	// Build polygon navmesh from the contours.
	rcPolyMesh* m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh) {
		LOG("[Recast tile]: Out of memory 'pmesh'.");
		return;
	}
	if (!rcBuildPolyMesh(&tile_ctx, *m_cset, tile_cfg.maxVertsPerPoly, *m_pmesh)) {
		LOG("[Recast tile]: Could not triangulate contours.");
		return;
	}

	// Build detail mesh.
	rcPolyMeshDetail* m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh) {
		LOG("[Recast tile]: Out of memory 'dmesh'.");
		return;
	}

	if (!rcBuildPolyMeshDetail(&tile_ctx, *m_pmesh, *m_chf,
		tile_cfg.detailSampleDist, tile_cfg.detailSampleMaxError,
		*m_dmesh)) {
		LOG("[Recast tile]: Could build polymesh detail.");
		return;
	}

	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeContourSet(m_cset);
	m_cset = 0;

	unsigned char* navData = 0;
	int navDataSize = 0;
	if (tile_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON) {
		if (m_pmesh->nverts >= 0xffff) {
			// The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
			LOG("[Recast tile]: Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
			return;
		}

		// Update poly flags from areas.
		for (int i = 0; i < m_pmesh->npolys; ++i) {
			if (m_pmesh->areas[i] == RC_WALKABLE_AREA) {
				m_pmesh->areas[i] = 0; // Id of the walkable area
			}
			m_pmesh->flags[i] = 1 << m_pmesh->areas[i];
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		params.offMeshConUserID = m_geom->getOffMeshConnectionId();
		params.offMeshConCount = m_geom->getOffMeshConnectionCount();
		params.walkableHeight = EngineApp->detour->agentHeight;
		params.walkableRadius = EngineApp->detour->agentRadius;
		params.walkableClimb = EngineApp->detour->stepHeight;
		params.tileX = tx;
		params.tileY = ty;
		params.tileLayer = 0;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = tile_cfg.cs;
		params.ch = tile_cfg.ch;
		params.buildBvTree = true;

		if (!EngineApp->detour->createNavMeshData(&params, &navData, &navDataSize)) {
			LOG("[Recast tile]: Could not build Detour navmesh.");
			return;
		}
	}

	tile_ctx.stopTimer(RC_TIMER_TOTAL);

	rcFreePolyMesh(m_pmesh);
	rcFreePolyMeshDetail(m_dmesh);
	*datasize = navDataSize;
	*data = navData;
}

void M_Recast::MarkOBBArea(const math::OBB& obb, unsigned char areaId, rcCompactHeightfield& chf)
{
}
