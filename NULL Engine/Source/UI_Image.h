#ifndef __UI_IMAGE_H__
#define __UI_IMAGE_H__

#include <string>

#include "UIElement.h"

class C_Canvas;

class NULL_API UI_Image : public UIElement
{
public:
	UI_Image(C_Canvas* canvas, Rect rect);
	~UI_Image();

	bool Update() override;
	bool CleanUp() override;

	static UIElementType GetType() { return UIElementType::IMAGE; }

	void RenderImage2D();
	void RenderImage3D();

public:

private:
};

#endif // !__UI_IMAGE_H__