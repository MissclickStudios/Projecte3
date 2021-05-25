#ifndef __C_UI_BUTTON_H__
#define __C_UI_BUTTON_H__

#include "C_Ui.h"
#include "Color.h"
#include "Spritesheet.h"

class R_Shader;

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
	Frame GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight);
private:
	UIButtonState state = UIButtonState::NONE;

	uint VAO;
	uint VBO;
	Color idle = { 0.97f, 0.76f, 0.58f, 1.0f };
	Color hovered = { 1.0f, 1.0f, 1.0f, 1.0f };
	Color pressed = { 1.0f, 0.4f, 0.19f, 1.0f };

	R_Shader* rShader;
	int pixelCoord[4];
	Frame textCoord = { 0, 0, 1, 1 };

	friend class E_Inspector;
};

#endif // !__C_UI_BUTTON_H__