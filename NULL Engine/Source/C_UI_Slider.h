#ifndef __C_UI_SLIDER_H__
#define __C_UI_SLIDER_H__

#include "C_Ui.h"
#include "Spritesheet.h"

class R_Shader;

class MISSCLICK_API C_UI_Slider : public C_UI
{
public:

	C_UI_Slider(GameObject* owner, Rect2D rect = { 0,0,0.32,0.08 });
	~C_UI_Slider();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void LoadBuffers();

	static inline ComponentType GetType() { return ComponentType::UI_SLIDER; }

	void HandleInput(C_UI** selectedUi) override;
	void Draw2D() override;
	void Draw3D() override;

	void InputValue(float* value, float maxValue = -1);

private:
	void ResetInput()override;
	Frame GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight);

private:
	unsigned int VAO;
	unsigned int VBO;

	R_Shader* rShader;
	int pixelCoord[16];
	Frame unhoverUnchecked;
	Frame hoverUnchecked;
	Frame unhoverChecked;
	Frame hoverChecked;

	bool hovered = false;
	int numRects = 1;
	float maxValue = 0;
	float* trackedVariable = nullptr;
	float offset = 0.1f;
	//int minValue = 0; ??

	friend class E_Inspector;
};

#endif // !__C_UI_SLIDER_H__