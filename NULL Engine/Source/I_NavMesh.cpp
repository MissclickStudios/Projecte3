#include "R_NavMesh.h"

#include "I_NavMesh.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "RecastNavigation/Detour/Include/DetourNavMesh.h"

bool Importer::NavMesh::Import(const char* buffer, R_NavMesh* rNavMesh)
{

	return true;
}

uint Importer::NavMesh::Save(const R_NavMesh* rNavMesh, char** buffer)
{
	uint written = 0;

	uint datasize = 0;
	char* data = nullptr;
	char* d_index = nullptr;
	NavMeshSetHeader* header = new NavMeshSetHeader();
	datasize += sizeof(NavMeshSetHeader);
	header->magic = NAVMESHSET_MAGIC;
	header->version = NAVMESHSET_VERSION;
	header->numTiles = 0;

	const dtNavMesh* mesh = rNavMesh->navMesh;
	for (int i = 0; i < mesh->getMaxTiles(); ++i) {
		const dtMeshTile* tile = (const dtMeshTile*)mesh->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;
		header->numTiles++;
		datasize += sizeof(NavMeshTileHeader);
		datasize += tile->dataSize;
	}

	memcpy(&header->params, mesh->getParams(), sizeof(dtNavMeshParams));

	data = new char[datasize];
	d_index = data;

	// We write the header file
	memcpy(d_index, header, sizeof(NavMeshSetHeader));
	d_index += sizeof(NavMeshSetHeader);

	// Store tiles.
	for (int i = 0; i < mesh->getMaxTiles(); ++i) {
		const dtMeshTile* tile = mesh->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		NavMeshTileHeader tileHeader;
		tileHeader.tileRef = mesh->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		memcpy(d_index, &tileHeader, sizeof(NavMeshTileHeader));
		d_index += sizeof(NavMeshTileHeader);

		memcpy(d_index, tile->data, tile->dataSize);
		d_index += tile->dataSize;
	}

	written = App->fileSystem->Save(rNavMesh->GetLibraryFile(), data, datasize);

	if (data) {
		delete[] data;
		data = nullptr;
		d_index = nullptr;
	}

	delete header;

	return written;
}

bool Importer::NavMesh::Load(const char* buffer, R_NavMesh* rNavMesh)
{

	return true;
}
