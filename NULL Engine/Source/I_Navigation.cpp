#include "R_NavMesh.h"

#include "I_Navigation.h"
#include "FileSystemDefinitions.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "RecastNavigation/Detour/Include/DetourNavMesh.h"

bool Importer::Navigation::Import(const char* buffer, R_NavMesh* rNavMesh)
{
	Load(buffer, rNavMesh);
	return true;
}

uint Importer::Navigation::Save(const R_NavMesh* rNavMesh, char** buffer)
{
	uint written = 0;
	
	uint size = 0;
	char* cursor = nullptr;
	NavMeshSetHeader* header = new NavMeshSetHeader();
	size += sizeof(NavMeshSetHeader);
	header->magic = NAVMESHSET_MAGIC;
	header->version = NAVMESHSET_VERSION;
	header->numTiles = 0;

	const dtNavMesh* mesh = rNavMesh->navMesh;
	for (int i = 0; i < mesh->getMaxTiles(); ++i) {
		const dtMeshTile* tile = (const dtMeshTile*)mesh->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;
		header->numTiles++;
		size += sizeof(NavMeshTileHeader);
		size += tile->dataSize;
	}

	memcpy(&header->params, mesh->getParams(), sizeof(dtNavMeshParams));

	*buffer = new char[size];
	cursor = *buffer;

	// We write the header file
	memcpy(cursor, header, sizeof(NavMeshSetHeader));
	cursor += sizeof(NavMeshSetHeader);

	// Store tiles.
	for (int i = 0; i < mesh->getMaxTiles(); ++i) {
		const dtMeshTile* tile = mesh->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		NavMeshTileHeader tileHeader;
		tileHeader.tileRef = mesh->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		memcpy(cursor, &tileHeader, sizeof(NavMeshTileHeader));
		cursor += sizeof(NavMeshTileHeader);

		memcpy(cursor, tile->data, tile->dataSize);
		cursor += tile->dataSize;
	}

	cursor = nullptr;
	delete header;

	// --- SAVING THE BUFFER ---
	std::string path = NAVIGATION_PATH + std::to_string(rNavMesh->GetUID()) + NAVMESH_EXTENSION;
	written = App->fileSystem->Save(path.c_str(), *buffer, size);

	std::string pathAss = ASSETS_NAVIGATION_PATH + rNavMesh->navMeshName + NAVMESH_AST_EXTENSION;
	App->fileSystem->Save(pathAss.c_str(), *buffer, size);

	return written;
}

bool Importer::Navigation::Load(const char* buffer, R_NavMesh* rNavMesh)
{
	bool ret = true;
	
	char* cursor = (char*)buffer;
	// Read header.
	NavMeshSetHeader header;
	errno_t readRes = memcpy_s(&header, sizeof(NavMeshSetHeader), cursor, sizeof(NavMeshSetHeader));
	cursor += sizeof(NavMeshSetHeader);

	if (readRes != 0)
		return false;

	if (header.magic != NAVMESHSET_MAGIC)
		return false;

	if (header.version != NAVMESHSET_VERSION)
		return false;

	rNavMesh->navMesh = dtAllocNavMesh();
	if (!rNavMesh->navMesh)
		return false;

	dtNavMeshParams params;
	memcpy(&params, &header.params, sizeof(dtNavMeshParams));

	dtStatus status = rNavMesh->navMesh->init(&params);
	if (dtStatusFailed(status))
		return false;

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i) {
		NavMeshTileHeader tileHeader;
		readRes = memcpy_s(&tileHeader, sizeof(NavMeshTileHeader), cursor, sizeof(NavMeshTileHeader));
		cursor += sizeof(NavMeshTileHeader);
		if (readRes != 0)
			return false;

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		readRes = memcpy_s(data, tileHeader.dataSize, cursor, tileHeader.dataSize);
		cursor += tileHeader.dataSize;
		if (readRes != 0) {
			dtFree(data);
			return false;
		}

		rNavMesh->navMesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	return ret;
}
