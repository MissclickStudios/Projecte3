#ifndef __C_UI_BUTTON_H__
#define __C_UI_BUTTON_H__

#include <string>
#include "C_Ui.h"

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
	PRESSEDIN,
	PRESSED,
	RELEASED
};

class MISSCLICK_API C_UI_Button : public C_UI
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

	void HandleInput(C_UI** selectedUi) override;
	void Draw2D() override;
	void Draw3D() override;

public:
	UIButtonState GetState()const;

private:
	void ResetInput()override;

private:
	UIButtonState state = UIButtonState::NONE;

	uint VAO;
	uint VBO;

	R_Shader* rShader;
};

#endif // !__C_UI_BUTTON_H__