#include "M_Detour.h"

#include "RecastNavigation/Detour/Include/DetourNavMeshQuery.h"
#include "RecastNavigation/Detour/Include/DetourNavMeshBuilder.h"
#include "RecastNavigation/Recast/Include/Recast.h"
#include "RecastNavigation/Detour/Include/DetourCommon.h"

#include "FileSystemDefinitions.h"

#include "Application.h"

#include "M_ResourceManager.h"
#include "M_Scene.h"
#include "M_Renderer3D.h"
#include "GameObject.h"
#include "C_Mesh.h"
#include "C_Transform.h"

#include "R_Material.h"
#include "R_Mesh.h"
#include "R_NavMesh.h"


M_Detour::M_Detour(bool isActive) : Module("Detour", isActive)
{
}

M_Detour::~M_Detour()
{
}

bool M_Detour::Init(ParsonNode& root)
{
	for (int i = 0; i < BE_DETOUR_TOTAL_AREAS; ++i) {
		sprintf_s(areaNames[i], "");
		areaCosts[i] = 1;
	}

	//Inititalize names
	sprintf_s(areaNames[0], "Walkable");
	sprintf_s(areaNames[1], "Not Walkable");
	sprintf_s(areaNames[2], "Jump");

	//Change value of Jump to 2 by default
	areaCosts[2] = 2;

	agentRadius = 0.5f;
	agentHeight = 2.0f;
	maxSlope = 45.0f;
	stepHeight = 0.4f;

	m_navQuery = dtAllocNavMeshQuery();
	m_filterQuery = new dtQueryFilter();

	// Set default size of box around points to look for nav polygons
	m_Extents[0] = 32.0f; m_Extents[1] = 32.0f; m_Extents[2] = 32.0f;

	return true;
}

bool M_Detour::CleanUp()
{
	return true;
}

void M_Detour::Draw() const
{
	if (debugDraw && navMeshResource != nullptr && navMeshResource->navMesh != nullptr) {
		for (int i = 0; i < renderMeshes.size(); ++i)
		{
		
		}
	}
}

void M_Detour::setDebugDraw(bool state)
{
	debugDraw = state;
}

bool M_Detour::createNavMesh(dtNavMeshCreateParams* params)
{
	LOG("Starting NavMesh build");
	unsigned char* navData = 0;
	int navDataSize = 0;

	dtNavMesh* m_navMesh = nullptr;
	if (navMeshResource != nullptr) {
		m_navMesh = navMeshResource->navMesh;
		navMeshResource->navMesh = nullptr;
	}

	if (m_navMesh != nullptr)
		dtFreeNavMesh(m_navMesh);
	m_navMesh = nullptr;


	if (!dtCreateNavMeshData(params, &navData, &navDataSize)) {
		LOG("Could not build Detour navmesh.");
		return false;
	}

	m_navMesh = dtAllocNavMesh();
	if (!m_navMesh) {
		dtFree(navData);
		m_navMesh = nullptr;
		LOG("Could not create Detour navmesh");
		return false;
	}

	dtStatus status;

	status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status)) {
		dtFree(navData);
		LOG("Could not init Detour navmesh");
		return false;
	}

	status = m_navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status)) {
		LOG("Could not init Detour navmesh query");
		return false;
	}

	if (navMeshResource == nullptr)
	{
		std::string resourceName = App->scene->GetCurrentScene();
		resourceName = ASSETS_NAVIGATION_PATH + resourceName + NAVMESH_AST_EXTENSION;

		navMeshResource = (R_NavMesh*)App->resourceManager->CreateResource(ResourceType::NAVMESH, resourceName.c_str());

		navMeshResource->SetNavMeshName(App->scene->GetCurrentScene());
	}
	navMeshResource->navMesh = m_navMesh;
	navMeshResource->SetNavMeshName(App->scene->GetCurrentScene());

	//Save Importer navmesh 
	App->resourceManager->SaveResourceToLibrary(navMeshResource);

	//If we are in the editor we need to create the draw meshes
	if (App->gameState == GameState::STOP) {
		createRenderMeshes();
	}

	return true;
}

void M_Detour::loadNavMeshFile(unsigned int navMeshUid, const char* navMeshPath)
{
	//App->resourceManager->AllocateResource(navMeshUid, navMeshPath);
	//navMeshResource = (R_NavMesh*) App->resourceManager->RequestResource(navMeshUid);

	navMeshResource = App->resourceManager->GetResource<R_NavMesh>(navMeshPath);

	if (navMeshResource != nullptr)
	{
		if (navMeshResource->navMesh != nullptr)
		{
			dtStatus status = m_navQuery->init(navMeshResource->navMesh, 2048);
			if (dtStatusFailed(status)) {
				LOG("Could not init Detour navmesh query");
			}
			LOG("%d", navMeshResource->navMesh->getMaxTiles());
		}
	}
	
}

void M_Detour::deleteNavMesh()
{
}

void M_Detour::clearNavMesh()
{
	navMeshResource = nullptr;
	for (int i = 0; i < renderMeshes.size(); ++i)
		delete renderMeshes[i];
	renderMeshes.clear();
}

void M_Detour::setDefaultValues()
{
	for (int i = 3; i < BE_DETOUR_TOTAL_AREAS; ++i) {
		sprintf_s(areaNames[i], "");
		areaCosts[i] = 1;
	}

	//Inititalize names
	sprintf_s(areaNames[0], "Walkable");
	sprintf_s(areaNames[1], "Not Walkable");
	sprintf_s(areaNames[2], "Jump");

	//Change value of Jump to 2 by default
	areaCosts[2] = 2;

	setDefaultBakeValues();
	setAreaCosts();
}

void M_Detour::setDefaultBakeValues()
{
	agentRadius = 0.5f;
	agentHeight = 2.0f;
	maxSlope = 45.0f;
	stepHeight = 0.4f;
	voxelSize = 0.15f;
	voxelHeight = 0.2f;

	regionMinSize = 8;
	regionMergeSize = 20;
	edgeMaxLen = 12.0f;
	edgeMaxError = 1.3f;
	vertsPerPoly = 6.0f;
	detailSampleDist = 6.0f;
	detailSampleMaxError = 1.0f;

	buildTiledMesh = true;
}

const R_NavMesh* M_Detour::getNavMeshResource() const
{
	return navMeshResource;
}

void M_Detour::allocateNavMesh()
{
	if (navMeshResource == nullptr)
	{
		std::string resourceName = App->scene->GetCurrentScene();
		resourceName = ASSETS_NAVIGATION_PATH + resourceName + NAVMESH_AST_EXTENSION;

		navMeshResource = (R_NavMesh*)App->resourceManager->CreateResource(ResourceType::NAVMESH, resourceName.c_str());
		navMeshResource->SetNavMeshName(App->scene->GetCurrentScene());
	}

	if (navMeshResource->navMesh != nullptr)
		dtFreeNavMesh(navMeshResource->navMesh);


	navMeshResource->navMesh = dtAllocNavMesh();
	navMeshResource->SetNavMeshName(App->scene->GetCurrentScene());

}

void M_Detour::createRenderMeshes()
{
	if (navMeshResource != nullptr && navMeshResource->navMesh != nullptr) {
		for (int i = 0; i < renderMeshes.size(); ++i)
			delete renderMeshes[i];
		renderMeshes.clear();

		const dtNavMesh* mesh = navMeshResource->navMesh;
		for (int ti = 0; ti < mesh->getMaxTiles(); ++ti) {
			const dtMeshTile* tile = mesh->getTile(ti);
			if (!tile->header) continue;
			processTile(tile);
		}
		LoadNavMeshBuffers(renderMeshes);
	}
}

void M_Detour::saveNavMesh() const
{
	App->resourceManager->SaveResourceToLibrary(navMeshResource);
}

inline void M_Detour::initNavQuery()
{
	if (navMeshResource != nullptr && navMeshResource->navMesh != nullptr)
		m_navQuery->init(navMeshResource->navMesh, 2048);
}

bool M_Detour::createNavMeshData(dtNavMeshCreateParams* params, unsigned char** outData, int* outDataSize)
{
	return dtCreateNavMeshData(params, outData, outDataSize);
}

unsigned int M_Detour::initNavMesh(const dtNavMeshParams* params)
{
	uint ret = DT_FAILURE;
	if (navMeshResource != nullptr && navMeshResource->navMesh != nullptr)
		ret = navMeshResource->navMesh->init(params);

	return ret;
}

void M_Detour::freeNavMeshData(void* ptr)
{
	dtFree(ptr);
}

void M_Detour::setAreaCosts()
{
	if (m_filterQuery != nullptr) {
		m_filterQuery->setAreaCost(0, areaCosts[0]);
		for (int i = 2; i < BE_DETOUR_TOTAL_AREAS; ++i)
			m_filterQuery->setAreaCost(i, areaCosts[i]);
	}
}

void M_Detour::processTile(const dtMeshTile* tile)
{
	for (int i = 0; i < tile->header->polyCount; ++i) {
		const dtPoly* poly = &tile->polys[i];
		if (poly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
			continue;

		const dtPolyDetail* poly_d = &tile->detailMeshes[i];
		navigationPoly* navpol = new navigationPoly();


		navpol->color = areaToColor(poly->getArea());
		int size = (tile->header->vertCount + tile->header->detailVertCount);
		navpol->rmesh->vertices.resize(size*3);
		//navpol->rmesh->vertices.reserve(navpol->rmesh->vertices.size()/3);

		float* vertData = navpol->rmesh->vertices.data();
		// We copy the vertices
		for (int j = 0; j < size; ++j) {
			float* vert;
			if (j < tile->header->vertCount)
				vert = &tile->verts[j * 3];
			else
				vert = &tile->detailVerts[(poly_d->vertBase + j - tile->header->vertCount) * 3];

			memcpy(&vertData[j*3], vert, sizeof(float) * 3);
			// @Improvement This is a fix for the navmesh render collinding with the geometry, might need to fix the renderer
			//navpol->rmesh->vertices[j].position[1] += 0.1f;
		}

		navpol->rmesh->indices.resize(poly_d->triCount * 3);
		//navpol->rmesh->indices.reserve(navpol->rmesh->indices.size());
		// Index pointer to copy the indices
		uint* index_indices = navpol->rmesh->indices.data();
		for (int j = 0; j < poly_d->triCount; ++j) {
			const unsigned char* t = &tile->detailTris[(poly_d->triBase + j) * 4];
			for (int k = 0; k < 3; ++k) {
				if (t[k] < poly->vertCount)
					(*index_indices) = poly->verts[t[k]];
				else
					(*index_indices) = t[k] - poly->vertCount + tile->header->vertCount;
				index_indices++;

			}
		}

		//navpol->rmesh.
		renderMeshes.push_back(navpol);
	}
	//TO draw tile per tile?
	//LoadNavMeshBuffer(renderMeshes);
}

inline int bit(int a, int b) {
	return (a & (1 << b)) >> b;
}

Color M_Detour::areaToColor(unsigned int area) const
{
	if (area == 0)
		return Color(0, 192, 255, 255);
	else {
		int	r = bit(area, 1) + bit(area, 3) * 2 + 1;
		int	g = bit(area, 2) + bit(area, 4) * 2 + 1;
		int	b = bit(area, 0) + bit(area, 5) * 2 + 1;

		return Color(r * 63, g * 63, b * 63, 255);
	}
}

navigationPoly::navigationPoly() {
	rmesh = new R_Mesh();
}

navigationPoly::~navigationPoly() {
	if (rmesh) {
		rmesh->CleanUp();
		delete rmesh;
	}
}

//TODO: maybe put everything together in 1 buffer will increase performance
void M_Detour::LoadNavMeshBuffers(std::vector<navigationPoly*>& meshes)
{
	for (int i = 0; i != meshes.size(); ++i)
	{
		meshes[i]->rmesh->LoadStaticBuffers();
	}
}

// Scripting
inline bool inRange(const float* v1, const float* v2, const float r, const float h)
{
	const float dx = v2[0] - v1[0];
	const float dy = v2[1] - v1[1];
	const float dz = v2[2] - v1[2];
	return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
}

static int fixupCorridor(dtPolyRef* path, const int npath, const int maxPath,
	const dtPolyRef* visited, const int nvisited)
{
	int furthestPath = -1;
	int furthestVisited = -1;

	// Find furthest common polygon.
	for (int i = npath - 1; i >= 0; --i)
	{
		bool found = false;
		for (int j = nvisited - 1; j >= 0; --j)
		{
			if (path[i] == visited[j])
			{
				furthestPath = i;
				furthestVisited = j;
				found = true;
			}
		}
		if (found)
			break;
	}

	// If no intersection found just return current path. 
	if (furthestPath == -1 || furthestVisited == -1)
		return npath;

	// Concatenate paths.	

	// Adjust beginning of the buffer to include the visited.
	const int req = nvisited - furthestVisited;
	const int orig = rcMin(furthestPath + 1, npath);
	int size = rcMax(0, npath - orig);
	if (req + size > maxPath)
		size = maxPath - req;
	if (size)
		memmove(path + req, path + orig, size * sizeof(dtPolyRef));

	// Store visited
	for (int i = 0; i < req; ++i)
		path[i] = visited[(nvisited - 1) - i];

	return req + size;
}

// This function checks if the path has a small U-turn, that is,
// a polygon further in the path is adjacent to the first polygon
// in the path. If that happens, a shortcut is taken.
// This can happen if the target (T) location is at tile boundary,
// and we're (S) approaching it parallel to the tile edge.
// The choice at the vertex can be arbitrary, 
//  +---+---+
//  |:::|:::|
//  +-S-+-T-+
//  |:::|   | <-- the step can end up in here, resulting U-turn path.
//  +---+---+
static int fixupShortcuts(dtPolyRef* path, int npath, dtNavMeshQuery* navQuery)
{
	if (npath < 3)
		return npath;

	// Get connected polygons
	static const int maxNeis = 16;
	dtPolyRef neis[maxNeis];
	int nneis = 0;

	const dtMeshTile* tile = 0;
	const dtPoly* poly = 0;
	if (dtStatusFailed(navQuery->getAttachedNavMesh()->getTileAndPolyByRef(path[0], &tile, &poly)))
		return npath;

	for (unsigned int k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
	{
		const dtLink* link = &tile->links[k];
		if (link->ref != 0)
		{
			if (nneis < maxNeis)
				neis[nneis++] = link->ref;
		}
	}

	// If any of the neighbour polygons is within the next few polygons
	// in the path, short cut to that polygon directly.
	static const int maxLookAhead = 6;
	int cut = 0;
	for (int i = dtMin(maxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
		for (int j = 0; j < nneis; j++)
		{
			if (path[i] == neis[j]) {
				cut = i;
				break;
			}
		}
	}
	if (cut > 1)
	{
		int offset = cut - 1;
		npath -= offset;
		for (int i = 1; i < npath; i++)
			path[i] = path[i + offset];
	}

	return npath;
}

int M_Detour::getAreaCost(unsigned int areaIndex) const
{
	if (areaIndex < BE_DETOUR_TOTAL_AREAS)
		return areaCosts[areaIndex];
	else return -1;
}

void M_Detour::setAreaCost(unsigned int areaIndex, float areaCost)
{
	if (m_filterQuery != nullptr) {
		m_filterQuery->setAreaCost(0, areaCosts[0]);
		for (int i = 2; i < BE_DETOUR_TOTAL_AREAS; ++i)
			m_filterQuery->setAreaCost(i, areaCosts[i]);
	}
}

int M_Detour::getAreaFromName(const char* name) const
{
	int ret = -1;
	std::string areaName = name;
	for (int i = 0; i < BE_DETOUR_TOTAL_AREAS; ++i) {
		if (areaName == areaNames[i]) {
			ret = i;
			break;
		}
	}

	return ret;
}

int M_Detour::calculatePath(float3 sourcePosition, float3 destination, int areaMask, std::vector<float3>& path)
{
	m_filterQuery->setIncludeFlags(areaMask);
	dtStatus status;
	dtPolyRef StartPoly;
	float StartNearest[3];
	dtPolyRef EndPoly;
	float EndNearest[3];
	dtPolyRef PolyPath[BE_DETOUR_MAX_PATHPOLY];
	int nPathCount = 0;
	float StraightPath[BE_DETOUR_MAX_PATHVERT * 3];
	int nVertCount = 0;


	// find the start polygon
	status = m_navQuery->findNearestPoly(sourcePosition.ptr(), m_Extents, m_filterQuery, &StartPoly, StartNearest);
	if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -1; // couldn't find a polygon

	// find the end polygon
	status = m_navQuery->findNearestPoly(destination.ptr(), m_Extents, m_filterQuery, &EndPoly, EndNearest);
	if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -2; // couldn't find a polygon

	status = m_navQuery->findPath(StartPoly, EndPoly, StartNearest, EndNearest, m_filterQuery, PolyPath, &nPathCount, BE_DETOUR_MAX_PATHPOLY);
	if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -3; // couldn't create a path
	if (nPathCount == 0) return -4; // couldn't find a path

	status = m_navQuery->findStraightPath(StartNearest, EndNearest, PolyPath, nPathCount, StraightPath, NULL, NULL, &nVertCount, BE_DETOUR_MAX_PATHVERT);
	if ((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)) return -5; // couldn't create a path
	if (nVertCount == 0) return -6; // couldn't find a path

	path.resize(nVertCount);
	memcpy(path.data(), StraightPath, sizeof(float) * nVertCount * 3);

	return nVertCount;
}

bool M_Detour::nearestPosInMesh(float3 sourcePosition, int areaMask, float3& nearestPoint)
{
	m_filterQuery->setIncludeFlags(areaMask);
	dtPolyRef nearestPoly;
	dtStatus status;

	//Find the nearest point
	status = m_navQuery->findNearestPoly(sourcePosition.ptr(), m_Extents, m_filterQuery, &nearestPoly, nearestPoint.ptr());
	return !((status & DT_FAILURE) || (status & DT_STATUS_DETAIL_MASK)); //If we found an error we return false
}