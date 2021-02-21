#ifndef __UIE_BUTTON_H__
#define __UIE_BUTTON_H__

#include <string>

#include "UIElement.h"

class C_Canvas;
struct AnimationClip;

class UIE_Button : public UIElement
{
public:
	UIE_Button(C_Canvas* owner);
	~UIE_Button();

	bool Update() override;
	bool CleanUp() override;

	static UIElementType GetType() { return UIElementType::BUTTON; }

public:

private:

	std::string text;
};

#endif // !__C_ANIMATION_H__