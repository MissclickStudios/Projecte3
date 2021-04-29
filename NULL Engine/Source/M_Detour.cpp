#include "M_Detour.h"

#include "RecastNavigation/Detour/Include/DetourNavMeshQuery.h"
#include "RecastNavigation/Detour/Include/DetourNavMeshBuilder.h"

#include "Application.h"

#include "M_Scene.h"
#include "GameObject.h"
#include "C_Mesh.h"
#include "C_Transform.h"

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
}

void M_Detour::setDebugDraw(bool state)
{
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

	if (navMeshResource == nullptr) {
		// Create navMeshResource

	}
	navMeshResource->navMesh = m_navMesh;

	//Save Importer navmesh 
	/*ImporterNavMesh* INavMesh = App->resources->GetImporter<ImporterNavMesh>();
	INavMesh->Save(navMeshResource);*/

	//If we are in the editor we need to create the draw meshes
	if (App->gameState == GameState::STOP) {
		createRenderMeshes();
	}

	//We save the scene so that it stores the NavMesh
	//App->scene_manager->SaveScene(App->scene_manager->currentScene);

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
	navMeshResource = nullptr;
	for (int i = 0; i < renderMeshes.size(); ++i)
		delete renderMeshes[i];
	renderMeshes.clear();
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
}

void M_Detour::processTile(const dtMeshTile* tile)
{
	/*for (int i = 0; i < tile->header->polyCount; ++i) {
		const dtPoly* poly = &tile->polys[i];
		if (poly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
			continue;

		const dtPolyDetail* poly_d = &tile->detailMeshes[i];
		navigationPoly* navpol = new navigationPoly();


		navpol->color = areaToColor(poly->getArea());
		navpol->rmesh->VerticesSize = tile->header->vertCount + tile->header->detailVertCount;
		navpol->rmesh->vertices = new float[navpol->rmesh->VerticesSize];
		navpol->rmesh->IndicesSize = poly_d->triCount * 3;
		navpol->rmesh->Indices = new uint[navpol->rmesh->IndicesSize];


		// We copy the vertices
		for (int j = 0; j < navpol->rmesh->VerticesSize; ++j) {
			float* vert;
			if (j < tile->header->vertCount)
				vert = &tile->verts[j * 3];
			else
				vert = &tile->detailVerts[(poly_d->vertBase + j - tile->header->vertCount) * 3];

			memcpy(navpol->rmesh->vertices[j].position, vert, sizeof(float) * 3);
			// @Improvement This is a fix for the navmesh render collinding with the geometry, might need to fix the renderer
			navpol->rmesh->vertices[j].position[1] += 0.1f;
		}

		// Index pointer to copy the indices
		uint* index_indices = navpol->rmesh->Indices;
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

		//To create EBO and VBO
		navpol->rmesh->LoadInMemory();
		renderMeshes.push_back(navpol);
	}*/
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