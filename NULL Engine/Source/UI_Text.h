#ifndef __UI_TEXT_H__
#define __UI_TEXT_H__

#include <string>
#include "Globals.h"

#include "UIElement.h"

class C_Canvas;

class UI_Text : public UIElement
{
public:
	UI_Text(GameObject* owner, Rect rect = { 0,0,50,50 });
	~UI_Text();

	bool Update() override;
	bool CleanUp() override;

	static UIElementType GetType() { return UIElementType::TEXT; }

public:

	std::string GetText() const;
	uint GetFontSize() const;

	void SetText(const std::string& text);
	void SetFontSize(const uint size);

private:

	std::string text;
	uint fontSize;

};

#endif // !__UI_TEXT_H__