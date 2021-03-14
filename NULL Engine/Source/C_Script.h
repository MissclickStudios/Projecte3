#ifndef __C_SCRIPT_H__
#define __C_SCRIPT_H__

#include "Component.h"

namespace math
{
	class float3;
}

class ParsonNode;
class GameObject;

class NULL_API C_Script : public Component
{
public:
	C_Script(GameObject* owner);
	~C_Script();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::MESH; }			// This is needed to be able to use templates for functions such as GetComponent<>();

public:
	R_Mesh* GetMesh() const;								// Returns the R_Mesh* variable of the component. If there is no mesh the default value will be nullptr.
	void	SetMesh(R_Mesh* rMesh);						// Sets the R_Mesh* variable of the component. Will be normally set when a model is imported.

	const char* GetMeshPath() const;
	const char* GetMeshFile() const;
	void SetMeshPath(const char* path);

	void GetMeshData(uint& numVertices, uint& numNormals, uint& numTexCoords, uint& numIndices, uint& numBones);

	bool GetDrawVertexNormals() const;
	bool GetDrawFaceNormals() const;
	void SetDrawVertexNormals(const bool& setTo);
	void SetDrawFaceNormals(const bool& setTo);

	bool GetShowWireframe() const;
	void SetShowWireframe(const bool& setTo);

	void GetBoundingBoxVertices(math::float3* bbVertices) const;		// TODO: Kinda dirty, should be done elsewhere (?).
	bool GetShowBoundingBox() const;
	void SetShowBoundingBox(const bool& setTo);

private:

};

#endif // !__C_SCRIPT_H__