#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include <vector>
#include <cmath>

#include "Color.h"
#include "VariableTypedefs.h"
#include "MathGeoTransform.h"

enum class PRIMITIVE_TYPES
{
	P_POINT,
	LINE,
	PLANE,
	CUBE,
	SPHERE,
	CYLINDER,
	PYRAMID,
	NONE
};

enum class BUFFER_TYPE
{
	VERTICES,
	NORMALS,
	UVS,
	INDICES,
	MAX_BUFFER_TYPES
};

class Primitive
{
public:
	Primitive();

	void Update();

	virtual void	Render() const;
	virtual void	RenderByIndices();
	void			LoadBuffersOnMemory();

public:
	void			SetPos(float x, float y, float z);
	float3			GetPos() const;
	void			SetRotation(const float3 &u, const float& angle);		// Angle in Radiants.
	void			Scale(float x, float y, float z);
	PRIMITIVE_TYPES	GetType() const;

public:
	Color color;
	float4x4 transform;
	bool axis, wire;

	uint VAO;																// Defines the Vertex Array Object that will hold all buffer objects of a primitive.
	uint buffers[(uint)BUFFER_TYPE::MAX_BUFFER_TYPES];						// Will hold all the possible buffers that a given primitive might use.
	uint bufferSize[(uint)BUFFER_TYPE::MAX_BUFFER_TYPES];					// Will be used to define the size in bytes of a given buffer.

	std::vector<float>	vertices;
	std::vector<float>	normals;
	std::vector<float>	uvs;
	std::vector<uint>	indices;

protected:
	virtual void InnerRender() const;
	virtual void IndicesRender();

protected:
	PRIMITIVE_TYPES type;
};

// ============================================
class P_Cube : public Primitive
{
public :
	P_Cube(const float3& size = float3::one, float mass = 1.f);

	float3 GetSize() const;

public:
	void DirectRender() const;
	void ArrayRender();

	void ApplyTransform(float* coordinates, int array_size);
	void ApplySize(float* coordinates, int array_size);

protected:
	void IndicesRender() override;

	//void DirectRender() const;
	//void ArrayRender();
	//void IndicesRender();

private:
	float3 size;

	//unsigned int VAO;

	bool loaded_in_array;
	bool loaded_in_indices;
};

// ============================================
class P_Sphere : public Primitive
{
public:
	//Sphere(float radius = 1.f, float mass = 1.f);
	P_Sphere(float radius = 1.f, uint rings = 12, uint sectors = 24);

public:
	float GetRadius() const;
	uint GetRings() const;
	uint GetSectors() const;

	void SetRings(uint rings);
	void SetSectors(uint sectors);

	//void IndiceRender(float radius, const uint rings, const uint sectors);

protected:
	void InnerRender() const;
	void IndicesRender() override;

protected:
	bool loaded_buffers;

private:
	float radius;
	uint rings;
	uint sectors;
};

// ============================================
class P_Cylinder : public Primitive
{
public:
	P_Cylinder(float radius = 1.f, float height = 2.f, uint sectors = 6, float mass = 1.f);

	float GetRadius() const;
	float GetHeight() const;
	uint GetSectors() const;
	void SetSectors(uint sectors);

	void InnerRender() const;
	
	std::vector<float> GetCircularVertices();

	void ConstructCoverVertices(std::vector<float> circular_vertices);
	void ConstructBaseAndTopVertices(std::vector<float> circular_vertices);
	void ConstructIndices(uint base_center_index, uint top_center_index);

protected:
	//void InnerRender() const;
	void IndicesRender() override;

private:
	float radius;
	float height;
	uint sectors;

	bool loaded_in_buffers;
};

// ============================================
class P_Pyramid : public Primitive
{
public:
	P_Pyramid(float3 size = float3::one);

protected:
	void InnerRender() const override;
	void IndicesRender() override;

protected:
	//std::vector<float> vertices;
	//std::vector<uint> indices;

private:
	float3 size;

	bool loaded_in_buffers;
};

// ============================================
class P_Line : public Primitive
{
public:
	P_Line();
	P_Line(const float3& A, const float3& B);

	float3 GetOrigin() const;
	float3 GetDestination() const;

protected:
	void InnerRender() const;
	void IndicesRender() override;

public:
	float3 origin;
	float3 destination;
};

// ============================================
class P_Plane : public Primitive
{
public:
	P_Plane(const float3& normal = float3(0.0f, 1.0f, 0.0f));

	float3 GetNormal() const;

protected:
	void InnerRender() const;
	void IndicesRender() override;

private:
	float3 normal;
};

// ============================================
class PrimitiveDrawExamples : public Primitive
{
public:
	PrimitiveDrawExamples();
	PrimitiveDrawExamples(float size, float3 origin);

	void DrawAllExamples();								// Will draw all the examples. All examples are drawn in Direct Mode.

public:
	void GL_PointsExample			(uint index = 0);
	void GL_LinesExample			(uint index = 1);
	void GL_LineStripExample		(uint index = 2);
	void GL_LineLoopExample			(uint index = 3);
	void GL_PolygonExample			(uint index = 4);
	void GL_TrianglesExample		(uint index = 5);
	void GL_TriangleStripExample	(uint index = 6);
	void GL_TriangleFanExample		(uint index = 7);
	void GL_QuadsExample			(uint index = 8);
	void GL_QuadStripExample		(uint index = 9);

public:
	float3 origin;
	float size;											//Just used to scale everything uniformly.
};

#endif // !__PRIMITIVE_H__