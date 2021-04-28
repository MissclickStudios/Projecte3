#ifndef __C_CANVAS_H__
#define __C_CANVAS_H__

#include <vector>

#include "Component.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"

class ParsonNode;
class C_UI_Button;
class C_UI_Image;

struct MISSCLICK_API Rect2D
{
	float x, y, w, h;
};

class MISSCLICK_API C_Canvas : public Component
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

	bool CheckButtonStates(); // Returns false if no buttons are hovered/pressed
	
	void UpdateActiveButtons();

public:

	float2 GetPosition() const;
	float2 GetSize() const;
	Rect2D GetRect() const;
	bool IsInvisible() const;

	void SetPosition(const float2& postion);
	void SetSize(const float2& size);
	void SetRect(const Rect2D& rect);
	void SetIsInvisible(const bool setTo);

public:

	std::vector<C_UI_Button*> activeButtons;

	C_UI_Button* selectedButton = nullptr;
	C_UI_Button* hoveredButton = nullptr;

	int priority = 0;

private:
	C_UI_Button* buttonIterator = nullptr;

	Rect2D rect = { 0,0,50,50 };
	bool isInvisible; // This is not the same as active, this just prevents drawing
};

#endif // !__C_CANVAS_H__