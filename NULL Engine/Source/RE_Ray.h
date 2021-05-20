#ifndef __RE_RAY_H__
#define __RE_RAY_H__

#include "MathGeoLib/include/Geometry/LineSegment.h"

#include "Renderer.h"

class RE_Ray : public Renderer
{
public:
	RE_Ray(float4x4* transform, float lineWidth, Color color, const LineSegment& ray);
	RE_Ray(const LineSegment& ray);
	~RE_Ray();

	bool Render() override;
	bool CleanUp() override;

private:
	LineSegment ray;
};

#endif // !__RE_RAY_H__