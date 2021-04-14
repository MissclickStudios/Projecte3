#ifndef __C_UI_IMAGE_H__
#define __C_UI_IMAGE_H__

#include <string>

#include "Component.h"

const float texCoordsBuffer[] = {
-1, -1,
1, -1,
-1, 1,
1, -1,
1, 1,
-1, 1,
};

//const float vertices[] = {
//	// pos      // tex
//	0.0f, 1.0f, 0.0f, 1.0f,
//	1.0f, 0.0f, 1.0f, 0.0f,
//	0.0f, 0.0f, 0.0f, 0.0f,
//
//	0.0f, 1.0f, 0.0f, 1.0f,
//	1.0f, 1.0f, 1.0f, 1.0f,
//	1.0f, 0.0f, 1.0f, 0.0f
//};

class NULL_API C_UI_Image : public Component
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

	void Draw2D();
	void Draw3D();

public:

	Rect2D GetRect() const;

	void SetRect(const Rect2D& rect);
	void SetX(const float x);
	void SetY(const float y);
	void SetW(const float w);
	void SetH(const float h);

private:

	Rect2D rect = { 0,0,2,2 };

	uint VAO;
	uint VBO;

	R_Shader* rShader;
};

#endif // !__C_UI_IMAGE_H__