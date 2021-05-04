
#include "InputGeom.h"
#include "Recast/Include/Recast.h"

// -- Components --
#include "../GameObject.h"
#include "../Component.h"
#include "../C_Mesh.h"
#include "../C_Transform.h"

// -- Resources --
#include "../R_Mesh.h"

// -- Utilities --
#include "../Log.h"
#include "../mmgr/include/mmgr.h"

InputGeom::InputGeom(const std::vector<GameObject*>& srcMeshes, bool createChunky) :
	nverts(0),
	ntris(0),
	maxtris(0),
	bmin(nullptr),
	bmax(nullptr),
	m_offMeshConCount(0),
	m_volumeCount(0),
	normals(nullptr),
	verts(nullptr),
	tris(nullptr),
	areas(nullptr),
	m_mesh(false),
	chunkyTriMesh(nullptr)
{
	R_Mesh* r_mesh = nullptr;
	vec _bmin, _bmax;
	bool minmaxset = false;

	float4x4 transform;
	// First pass to allocate our arrays
	for (std::vector<GameObject*>::const_iterator it = srcMeshes.cbegin(); it != srcMeshes.cend(); ++it) {
		r_mesh = (*it)->GetComponent<C_Mesh>()->GetMesh();
		ntris += r_mesh->indices.size();
		nverts += r_mesh->vertices.size();
	}


	meshes.reserve(srcMeshes.size());
	verts = new float[nverts * 3];
	tris = new int[ntris];
	areas = new unsigned char[ntris / 3];

	unsigned int t_verts = 0;
	unsigned int t_indices = 0;
	unsigned int t_tris = 0;
	for (std::vector<GameObject*>::const_iterator it = srcMeshes.cbegin(); it != srcMeshes.cend(); ++it) {
		m_mesh = true;
		r_mesh = (*it)->GetComponent<C_Mesh>()->GetMesh();
		transform = (*it)->GetComponent<C_Transform>()->GetWorldTransform();
		OBB t_obb = r_mesh->aabb;
		t_obb.Transform(transform);
		AABB aabb = t_obb.MinimalEnclosingAABB();

		meshes.push_back(RecastMesh());

		// We add the index count
		meshes.back().tris = &tris[t_indices];
		meshes.back().ntris = r_mesh->indices.size() / 3;

		// We do need to add the amount of vertices to the vertex they are pointing
		for (int i = 0; i < r_mesh->indices.size(); ++i)
			meshes.back().tris[i] = r_mesh->indices[i] + t_verts;

		t_indices += r_mesh->indices.size();
		if (meshes.back().ntris > maxtris)
			maxtris = meshes.back().ntris;

		// we add the transformed vertices
		meshes.back().nverts = r_mesh->vertices.size();
		float* vert_index = &verts[t_verts * 3];
		for (int i = 0; i < r_mesh->vertices.size(); i+=3) {
			ApplyTransform(&r_mesh->vertices[i], transform, vert_index);
			vert_index += 3;
		}
		t_verts += r_mesh->vertices.size();

		if (!minmaxset) {
			minmaxset = true;
			_bmin = aabb.minPoint;
			_bmax = aabb.maxPoint;
		}
		else {
			_bmin = math::Min(_bmin, aabb.minPoint);
			_bmax = math::Max(_bmax, aabb.maxPoint);
		}
		meshes.back().area = (*it)->navigationArea;

		if (meshes.back().area == 0)
			meshes.back().area = RC_WALKABLE_AREA;
		else if (meshes.back().area == 1)
			meshes.back().area = RC_NULL_AREA;

		for (int i = 0; i < r_mesh->indices.size() / 3; ++i)
			areas[t_tris + i] = meshes.back().area;
		t_tris += r_mesh->indices.size() / 3;
	}

	if (m_mesh) {
		ntris /= 3;
		bmin = new float[3];
		memcpy(bmin, _bmin.ptr(), 3 * sizeof(float));
		bmax = new float[3];
		memcpy(bmax, _bmax.ptr(), 3 * sizeof(float));
	}

	if (createChunky) {
		chunkyTriMesh = new rcChunkyTriMesh();
		if (!rcCreateChunkyTriMesh(verts, tris, areas, ntris, 256, chunkyTriMesh))
			LOG("[InputGeo]: Could not create ChunkyTriMesh for tiling");
	}
}


InputGeom::~InputGeom() {
	if (bmin != nullptr)
		delete[] bmin;
	if (bmax != nullptr)
		delete[] bmax;
	if (normals != nullptr)
		delete[] normals;
	if (verts != nullptr)
		delete[] verts;
	if (tris != nullptr)
		delete[] tris;
	if (areas != nullptr)
		delete[] areas;
	if (chunkyTriMesh != nullptr)
		delete chunkyTriMesh;

	meshes.clear();
}

const std::vector<RecastMesh>& InputGeom::getMeshes() const {
	return meshes;
}

const float* InputGeom::getVerts() const {
	return verts;
}

uint InputGeom::getVertCount() const {
	return nverts;
}

int InputGeom::getTriCount() const {
	return ntris;
}

int InputGeom::getMaxTris() const {
	return maxtris;
}

const float* InputGeom::getNormals() const {
	return normals;
}

const float* InputGeom::getMeshBoundsMin() const {
	return bmin;
}

const float* InputGeom::getMeshBoundsMax() const {
	return bmax;
}

const rcChunkyTriMesh* InputGeom::getChunkyMesh() const {
	return chunkyTriMesh;
}

bool InputGeom::hasMesh() const {
	return m_mesh;
}

void InputGeom::ApplyTransform(float* vertex, const float4x4& transform, float ret[3]) {
	math::float3 original;
	memcpy(original.ptr(), vertex, sizeof(float) * 3);
	math::float3 globalVert = (transform * math::float4(vertex[0],
		vertex[1],
		vertex[2],
		1)).xyz();
	size_t a = sizeof(math::float3);

	ret[0] = globalVert[0];
	ret[1] = globalVert[1];
	ret[2] = globalVert[2];
}

RecastMesh::~RecastMesh() {
}
