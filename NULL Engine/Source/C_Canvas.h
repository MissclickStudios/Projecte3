#ifndef __C_CANVAS_H__
#define __C_CANVAS_H__

#include <vector>

#include "Component.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"

class ParsonNode;
class GameObject;
class UIElement;

struct Rect
{
	float x, y, w, h;
};

class NULL_API C_Canvas : public Component
{
public:
	C_Canvas(GameObject* owner);
	~C_Canvas();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::CANVAS; }

	void Draw2D();
	void Draw3D();

public:

	float2 GetPosition() const;
	float2 GetSize() const;
	Rect GetRect() const;
	bool IsInvisible() const;
	float GetZ() const;

	void SetPosition(const float2& postion);
	void SetSize(const float2& size);
	void SetRect(const Rect& rect);
	void SetIsInvisible(const bool setTo);
	void SetZ(const float& z);



	//Temp. here
	float2 pivot = { 0,0 };

	std::vector<UIElement*>	uiElements;

private:

	Rect rect = { 0,0,50,50 };
	float z = 0;
	bool isInvisible; // This is not the same as active, this just prevents drawing
};

#endif // !__C_CANVAS_H__