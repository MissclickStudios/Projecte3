#ifndef __C_MESH_H__
#define __C_MESH_H__

#include "Component.h"

namespace math
{
	class float3;
}

class ParsonNode;
class GameObject;
class R_Mesh;

class C_Mesh : public Component
{
public:
	C_Mesh(GameObject* owner);
	~C_Mesh();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline COMPONENT_TYPE GetType() { return COMPONENT_TYPE::MESH; }			// This is needed to be able to use templates for functions such as GetComponent<>();

public:
	R_Mesh*			GetMesh					() const;								// Returns the R_Mesh* variable of the component. If there is no mesh the default value will be nullptr.
	void			SetMesh					(R_Mesh* r_mesh);						// Sets the R_Mesh* variable of the component. Will be normally set when a model is imported.

	const char*		GetMeshPath				() const;
	const char*		GetMeshFile				() const;
	void			SetMeshPath				(const char* path);

	void			GetMeshData				(uint& num_vertices, uint& num_normals, uint& num_tex_coords, uint& num_indices, uint& num_bones);

	bool			GetDrawVertexNormals	() const;
	bool			GetDrawFaceNormals		() const;
	void			SetDrawVertexNormals	(const bool& set_to);
	void			SetDrawFaceNormals		(const bool& set_to);

	bool			GetShowWireframe		() const;
	void			SetShowWireframe		(const bool& set_to);

	void			GetBoundingBoxVertices	(math::float3* bb_vertices) const;		// TODO: Kinda dirty, should be done elsewhere (?).
	bool			GetShowBoundingBox		() const;
	void			SetShowBoundingBox		(const bool& set_to);

private:
	R_Mesh* r_mesh;

	bool show_wireframe;
	bool show_bounding_box;
};

#endif // !__C_MESH_H__