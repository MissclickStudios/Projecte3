#ifndef __RE_SKELETON_H__
#define __RE_SKELETON_H__

#include "MathGeoLib/include/Geometry/LineSegment.h"

#include "Renderer.h"

class RE_Skeleton : public Renderer
{
public:
	RE_Skeleton(float4x4* transform, float lineWidth, Color color, const std::vector<LineSegment>& bones);
	RE_Skeleton(const std::vector<LineSegment>& bones);
	~RE_Skeleton();

	bool Render() override;
	bool CleanUp() override;

private:
	std::vector<LineSegment> bones;
};

#endif // !__RE_SKELETON_H__