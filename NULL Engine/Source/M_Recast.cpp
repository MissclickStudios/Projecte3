#include "M_Recast.h"
#include "M_Scene.h"

#include "RecastNavigation/Detour/Include/DetourNavMeshBuilder.h"
#include "RecastNavigation/Detour/Include/DetourNavMesh.h"
#include "RecastNavigation/DebugUtils/Include/RecastDebugDraw.h"
#include "RecastNavigation/DebugUtils/Include/DetourDebugDraw.h"
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

M_Recast::M_Recast(bool isActive) : Module("Recast", isActive),
m_maxTiles(0),
m_maxPolysPerTile(0),
m_tileSize(32),
m_tileTriCount(0)
{
	m_ctx = new rcContext();
}

M_Recast::~M_Recast()
{
	if (m_ctx != nullptr)
	{
		delete m_ctx;
		m_ctx = nullptr;
	}
}

bool M_Recast::Init(ParsonNode& config)
{
	return true;
}

void M_Recast::AddGO(GameObject* go)
{
	if (go->isNavigable)
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
	delete[] m_triareas;
	m_triareas = nullptr;
	rcFreeHeightField(m_solid);
	m_solid = nullptr;
	rcFreeCompactHeightfield(m_chf);
	m_chf = nullptr;
	rcFreeContourSet(m_cset);
	m_cset = nullptr;
	rcFreePolyMesh(m_pmesh);
	m_pmesh = nullptr;
	rcFreePolyMeshDetail(m_dmesh);
	m_dmesh = nullptr;

	
	return true;
}

bool M_Recast::BuildNavMesh()
{
	bool ret = true;
	LOG("[Recast mesh]: Starting Nav Mesh build");

	if (NavigationGameObjects.empty()) 
	{
		std::vector<GameObject*>* objects = App->scene->GetGameObjects();
		for (std::vector<GameObject*>::const_iterator cit = (*objects).cbegin(); cit != (*objects).cend(); ++cit)
		{
			if ((*cit)->isNavigable) 
			{
				NavigationGameObjects.push_back((*cit));
				LOG("Added fff");
			}
		}
		if (NavigationGameObjects.empty()) 
		{
			LOG("[Recast mesh]: No input mesh");
			return false;
		}
	}

	InputGeom* m_geom = new InputGeom(NavigationGameObjects, EngineApp->detour->buildTiledMesh);

	if (!m_geom || !m_geom->getMeshes().data())
	{
		LOG("[Recast mesh]: There are no meshes");
		return false;
	}

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = App->detour->m_cellSize;
	m_cfg.ch = App->detour->m_cellHeight;
	m_cfg.walkableSlopeAngle = App->detour->maxSlopeAngle;
	m_cfg.walkableHeight = (int)ceilf(App->detour->agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(App->detour->agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(App->detour->agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(App->detour->edgeMaxLen / App->detour->m_cellSize);
	m_cfg.maxSimplificationError = App->detour->edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(App->detour->regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(App->detour->regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)App->detour->vertsPerPoly;
	m_cfg.tileSize = (int)m_tileSize;
	m_cfg.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
	m_cfg.width = m_cfg.tileSize + m_cfg.borderSize * 2;
	m_cfg.height = m_cfg.tileSize + m_cfg.borderSize * 2;
	m_cfg.detailSampleDist = App->detour->detailSampleDist < 0.9f ? 0 : App->detour->m_cellSize * App->detour->detailSampleDist;
	m_cfg.detailSampleMaxError = App->detour->m_cellHeight * App->detour->detailSampleMaxError;
	rcVcopy(m_cfg.bmin, m_geom->getMeshBoundsMin());
	rcVcopy(m_cfg.bmax, m_geom->getMeshBoundsMax());

	if (EngineApp->detour->buildTiledMesh)
		ret = BuildTiledNavMesh(m_geom);
	else
		ret = BuildSoloNavMesh(m_geom);

	delete m_geom;
	m_geom = nullptr;

	return ret;
}

bool M_Recast::BuildSoloNavMesh(const InputGeom* m_geom)
{	
	//
	// Step 1. Initialize build config.
	//
	if (!m_geom || !m_geom->getMeshes().data())
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Input mesh is not specified.");
		return false;
	}

	CleanUp();

	//
	// Step 1. Initialize build config.
	//
	
	// Reset build times gathering.
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
	m_solid = rcAllocHeightfield();
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
	m_triareas = new unsigned char[m_geom->getMaxTris()];
	if (!m_triareas) {
		LOG("[Recast mesh]: Out of memory 'm_triareas' (%d).", m_geom->getMaxTris());
		return false;
	}

	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	for (int i = 0; i < m_geom->getMeshes().size(); ++i) {

		memset(m_triareas, 0, m_geom->getMaxTris() * sizeof(unsigned char));
		// Modified recast method to set the area of the mesh directly instead of having to mark it later
		rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, m_geom->getVerts(), m_geom->getMeshes()[i].nverts, m_geom->getMeshes()[i].tris, m_geom->getMeshes()[i].ntris, m_triareas);
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
	m_chf = rcAllocCompactHeightfield();
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
	// Step 5. Trace and simplify region contours.f
	//

	// Create contours.
	m_cset = rcAllocContourSet();
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
	m_pmesh = rcAllocPolyMesh();
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

	m_dmesh = rcAllocPolyMeshDetail();
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

	const float* bmin = m_geom->getMeshBoundsMin();
	const float* bmax = m_geom->getMeshBoundsMax();
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, App->detour->m_cellSize, &gw, &gh);
	const int ts = (int)m_tileSize;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;

	// Max tiles and max polys affect how the tile IDs are caculated.
	// There are 22 bits available for identifying a tile and a polygon.
	int tileBits = rcMin((int)ilog2(nextPow2(tw * th)), 14);
	if (tileBits > 14) tileBits = 14;
	int polyBits = 22 - tileBits;
	m_maxTiles = 1 << tileBits;
	m_maxPolysPerTile = 1 << polyBits;

	dtNavMeshParams params;
	rcVcopy(params.orig, m_geom->getMeshBoundsMin());
	params.tileWidth = m_tileSize * App->detour->m_cellSize;
	params.tileHeight = m_tileSize * App->detour->m_cellSize;
	params.maxTiles = m_maxTiles;
	params.maxPolys = m_maxPolysPerTile;

	dtStatus status;

	status = EngineApp->detour->initNavMesh(&params);
	if (dtStatusFailed(status)) {
		LOG("[Recast mesh]: Could not init navmesh.");
		return false;
	}

	status = App->detour->m_navQuery->init(App->detour->navMeshResource->navMesh, 2048);
	if (dtStatusFailed(status))
	{
		LOG("[Recast mesh]: buildTiledNavigation: Could not init Detour navmesh query");
		return false;
	}

	const float tcs = m_tileSize * App->detour->m_cellSize;
	const int totalTiles = th * tw;
	std::vector<unsigned char*> data;
	std::vector<int> dataSize;
	data.resize(totalTiles);
	dataSize.resize(totalTiles);

	for (int y = 0; y < th; ++y) {
		for (int x = 0; x < tw; ++x) {

			m_lastBuiltTileBmin[0] = bmin[0] + x * tcs;
			m_lastBuiltTileBmin[1] = bmin[1];
			m_lastBuiltTileBmin[2] = bmin[2] + y * tcs;

			m_lastBuiltTileBmax[0] = bmin[0] + (x + 1) * tcs;
			m_lastBuiltTileBmax[1] = bmax[1];
			m_lastBuiltTileBmax[2] = bmin[2] + (y + 1) * tcs;

			int dataSize = 0;
			unsigned char* data = BuildTile(m_geom, x, y, m_lastBuiltTileBmin, m_lastBuiltTileBmax, dataSize);
			if (data)
			{
				// Remove any previous data (navmesh owns and deletes the data).
				m_navmesh->removeTile(m_navmesh->getTileRefAt(x, y, 0), 0, 0);
				// Let the navmesh own the data.
				dtStatus status = m_navmesh->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, 0);
				if (dtStatusFailed(status))
					dtFree(data);
			}
		}
	}

	// We save the navmesh, create its render meshes and initialize the navQuery
	EngineApp->detour->initNavQuery();
	EngineApp->detour->createRenderMeshes();
	EngineApp->detour->saveNavMesh();

	return true;
}

unsigned char* M_Recast::BuildTile(const InputGeom* m_geom, const int tx, const int ty, const float* bmin, const float* bmax, int& datasize)
{
	CleanUp();

	const float* verts = m_geom->getVerts();
	const int nverts = m_geom->getVertCount();
	const int ntris = m_geom->getTriCount();
	const rcChunkyTriMesh* chunkyMesh = m_geom->getChunkyMesh();

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

	rcVcopy(m_cfg.bmin, bmin);
	rcVcopy(m_cfg.bmax, bmax);

	m_cfg.bmin[0] -= m_cfg.borderSize * m_cfg.cs;
	m_cfg.bmin[2] -= m_cfg.borderSize * m_cfg.cs;
	m_cfg.bmax[0] += m_cfg.borderSize * m_cfg.cs;
	m_cfg.bmax[2] += m_cfg.borderSize * m_cfg.cs;

	// Reset build times gathering.

	// Start the build process.

	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid) {
		LOG("[Recast tile]: Failed to allocate solid heightfield.");
		return 0;
	}
	if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch)) {
		LOG("[Recast tile]: Could not create solid heightfield.");
		return 0;
	}

	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	m_triareas = new unsigned char[chunkyMesh->maxTrisPerChunk];
	if (!m_triareas) {
		LOG("[Recast tile]: Out of memory 'm_triareas' (%d).", chunkyMesh->maxTrisPerChunk);
		return 0;
	}

	float tbmin[2], tbmax[2];
	tbmin[0] = m_cfg.bmin[0];
	tbmin[1] = m_cfg.bmin[2];
	tbmax[0] = m_cfg.bmax[0];
	tbmax[1] = m_cfg.bmax[2];
	int cid[512];// TODO: Make grow when returning too many items.
	const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
	if (!ncid)
		return 0;

	int m_tileTriCount = 0;

	for (int i = 0; i < ncid; ++i) {
		const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
		const int* ctris = &chunkyMesh->tris[node.i * 3];
		const uchar* careas = &chunkyMesh->areas[node.i];
		const int nctris = node.n;

		m_tileTriCount += nctris;

		memset(m_triareas, 0, nctris * sizeof(unsigned char));
		rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle,
			verts, nverts, ctris, nctris, m_triareas);

		if (!rcRasterizeTriangles(m_ctx, verts, nverts, ctris, m_triareas, nctris, *m_solid, m_cfg.walkableClimb)) {
			LOG("[Recast tile]: Could not rasterize triangles of tile x: %d, y: %d", tx, ty);
			return 0;
		}
	}

	delete[] m_triareas;
	m_triareas = 0;

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf) {
		LOG("[Recast tile]:  Out of memory 'chf'.");
		return 0;
	}
	if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf)) {
		LOG("[Recast tile]: Could not build compact data.");
		return 0;
	}

	rcFreeHeightField(m_solid);
	m_solid = 0;

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf)) {
		LOG("[Recast tile]: Could not erode.");
		return 0;
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
	if (!rcBuildDistanceField(m_ctx, *m_chf)) {
		LOG("[Recast tile]: Could not build distance field.");
		return 0;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea)) {
		LOG("[Recast tile]: Could not build watershed regions.");
		return 0;
	}

	// Create contours.
	m_cset = rcAllocContourSet();
	if (!m_cset) {
		LOG("[Recast tile]: Out of memory 'cset'.");
		return 0;
	}
	if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset)) {
		LOG("[Recast tile]: Could not create contours.");
		return 0;
	}

	if (m_cset->nconts == 0) {
		return 0;
	}

	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh) {
		LOG("[Recast tile]: Out of memory 'pmesh'.");
		return 0;
	}
	if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh)) {
		LOG("[Recast tile]: Could not triangulate contours.");
		return 0;
	}

	// Build detail mesh.
	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh) {
		LOG("[Recast tile]: Out of memory 'dmesh'.");
		return 0;
	}

	if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf,
		m_cfg.detailSampleDist, m_cfg.detailSampleMaxError,
		*m_dmesh)) {
		LOG("[Recast tile]: Could build polymesh detail.");
		return 0;
	}

	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeContourSet(m_cset);
	m_cset = 0;

	unsigned char* navData = 0;
	int navDataSize = 0;
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON) {
		if (m_pmesh->nverts >= 0xffff) {
			// The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
			LOG("[Recast tile]: Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
			return 0;
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
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		if (!EngineApp->detour->createNavMeshData(&params, &navData, &navDataSize)) {
			LOG("[Recast tile]: Could not build Detour navmesh.");
			return 0;
		}
	}

	datasize = navDataSize;
	return navData;
}

void M_Recast::MarkOBBArea(const math::OBB& obb, unsigned char areaId, rcCompactHeightfield& chf)
{
}
