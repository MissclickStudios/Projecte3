#ifndef __C_UI_BUTTON_H__
#define __C_UI_BUTTON_H__

#include <string>

#include "Component.h"

class R_Shader;

const float coordsBuffer[] = {
1, 1,
1, 0,
0, 0,
1,0,
};

enum class UIButtonState
{
	NONE = 0,
	IDLE,
	HOVERED,
	PRESSED,
	RELEASED
};

class MISSCLICK_API C_UI_Button : public Component
{
public:

	C_UI_Button(GameObject* owner, Rect2D rect = { 0,0,0.32,0.08 });
	~C_UI_Button();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void LoadBuffers();

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

	UIButtonState state = UIButtonState::NONE;
	Rect2D rect = { 0,0,0.32,0.08 };

	bool isPressed = false;

	bool isInit = false;

	uint VAO;
	uint VBO;

	R_Shader* rShader;
};

#endif // !__C_UI_BUTTON_H__