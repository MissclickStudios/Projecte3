#ifndef __C_UI_CHECKBOX_H__
#define __C_UI_CHECKBOX_H__

#include "C_Ui.h"

class R_Shader;

enum class UICheckboxState
{
	NONE = 0,
	CHECKED,
	UNCHECKED,
	HOVERED_CHECKED,
	HOVERED_UNCHECKED,
	PRESSED_CHECKED_IN,
	PRESSED_CHECKED,
	PRESSED_CHECKED_OUT,
	PRESSED_UNCHECKED_IN,
	PRESSED_UNCHECKED,
	PRESSED_UNCHECKED_OUT,
};

class MISSCLICK_API C_UI_Checkbox : public C_UI
{
public:

	C_UI_Checkbox(GameObject* owner, Rect2D rect = { 0,0,0.32,0.08 });
	~C_UI_Checkbox();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;
	void SetChecked();
	void SetUnchecked();

	void LoadBuffers();

	static inline ComponentType GetType() { return ComponentType::UI_CHECKBOX; }

	void HandleInput(C_UI** selectedUi) override;
	void Draw2D() override;
	void Draw3D() override;
	void Check();
	void UnCheck();

public:
	UICheckboxState GetState()const;

private:
	void ResetInput()override;
	const char* NameFromState(UICheckboxState state);

private:
	UICheckboxState state = UICheckboxState::UNCHECKED;

	unsigned int VAO;
	unsigned int VBO;

	int pixelCoord[24];
	Frame unhoverUnchecked;
	Frame hoverUnchecked;
	Frame unhoverChecked;
	Frame hoverChecked;
	Frame pressedChecked;
	Frame pressedUnchecked;

	R_Shader* rShader;

	friend class E_Inspector;
};

#endif // !__C_UI_CHECKBOX_H__