#ifndef __C_UI_BUTTON_H__
#define __C_UI_BUTTON_H__

#include <string>

#include "Component.h"

enum class UIButtonState
{
	IDLE,
	HOVERED,
	PRESSED
};

class NULL_API C_UI_Button : public Component
{
public:

	C_UI_Button(GameObject* owner, Rect2D rect = { 0,0,0.3,0.05 });

	~C_UI_Button();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::UI_BUTTON; }

	void Draw2D();
	void Draw3D();

	void OnPressed();
	void OnReleased();

public:

	Rect2D GetRect() const;
	UIButtonState GetState() const;
	bool IsPressed() const;

	void SetRect(const Rect2D& rect);
	void SetState(const UIButtonState& setTo);
	void SetIsPressed(const bool& setTo);
	void SetX(const float x);
	void SetY(const float y);
	void SetW(const float w);
	void SetH(const float h);

private:

	UIButtonState state = UIButtonState::IDLE;
	Rect2D rect = { 0,0,0.3,0.05 };

	bool isPressed = false;

};

#endif // !__C_UI_BUTTON_H__