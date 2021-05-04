#include "R_NavMesh.h"

#include "I_Navigation.h"
#include "FileSystemDefinitions.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "RecastNavigation/Detour/Include/DetourNavMesh.h"

bool Importer::Navigation::Import(const char* buffer, R_NavMesh* rNavMesh)
{

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

	return written;
}

bool Importer::Navigation::Load(const char* buffer, R_NavMesh* rNavMesh)
{

	return true;
}
