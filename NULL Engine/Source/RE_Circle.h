#ifndef __RE_CIRCLE_H__
#define __RE_CIRCLE_H__

#include <vector>

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

#include "Renderer.h"

struct Color;

typedef unsigned int uint;

class RE_Circle : public Renderer
{
public:
	RE_Circle(float4x4* transform, float radius, uint sectors, float lineWidth = 1.0f, Color color = Color(1.0f, 1.0f, 1.0f, 1.0f));
	RE_Circle(float4x4* transform, float radius, uint sectors);
	~RE_Circle();

	bool Render() override;
	bool CleanUp() override;

private:
	void CalculateVertices();

private:
	//float4x4 transform;
	float	radius;												// Distance from the center to the edge of the circle.
	uint	sectors;											// Amount of vertices that compose the circle. Ex: sectors = 6 --> hexagon.

	float*	vertices;											// Position of the circle's vertices. 1 vertex = 3 vertices.
};

#endif // !__RE_CIRCLE_H__