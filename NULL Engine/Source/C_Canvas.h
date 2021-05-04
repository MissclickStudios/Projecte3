#ifndef __C_CANVAS_H__
#define __C_CANVAS_H__

#include <vector>

#include "C_UI.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"

class ParsonNode;

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

	void HandleInput();
	void Draw2D(bool renderCavas);
	void Draw3D(bool renderCanvas);


	void RemoveUiElement(C_UI*element);
	void ResetUi();
	float2 GetPosition() const;
	float2 GetSize() const;
	Rect2D GetRect() const;

	void SetPosition(const float2& postion);
	void SetSize(const float2& size);
	void SetRect(const Rect2D& rect);

private:
	Rect2D rect = { 0,0,50,50 };
	C_UI* selectedUi = nullptr;
	std::vector<C_UI*> uiElements;

	
};

#endif // !__C_CANVAS_H__