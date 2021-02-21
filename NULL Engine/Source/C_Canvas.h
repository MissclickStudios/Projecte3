#ifndef __C_CANVAS_H__
#define __C_CANVAS_H__

#include <vector>

#include "Component.h"
#include "UIElement.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"

class ParsonNode;
class GameObject;
struct AnimationClip;

struct Rect
{
	float x, y, z, w, h;
};

class C_Canvas : public Component
{
public:
	C_Canvas(GameObject* owner);
	~C_Canvas();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::CANVAS; }

public:

	float3 GetPosition() const;
	float2 GetSize() const;
	Rect GetRect() const;

	void SetPosition(const float3& postion);
	void SetSize(const float2& size);
	void SetRect(const Rect& rect);

private:
	std::vector<UIElement*>	uiElements;

	Rect rect = { 0,0,0,1,1 };
};

#endif // !__C_CANVAS_H__