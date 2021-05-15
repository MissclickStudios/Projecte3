#ifndef __C_UI_IMAGE_H__
#define __C_UI_IMAGE_H__

#include <string>

#include "C_UI.h"


const float texCoordsBuffer[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
};


class MISSCLICK_API C_UI_Image : public C_UI
{
public:

	C_UI_Image(GameObject* owner, Rect2D rect = { 0,0,50,50 });

	~C_UI_Image();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::UI_IMAGE; }

	void LoadBuffers();

	void HandleInput(C_UI** selectedUi)override;
	void Draw2D()override;
	void Draw3D()override;

private:
	uint VAO;
	uint VBO;
};

#endif // !__C_UI_IMAGE_H__