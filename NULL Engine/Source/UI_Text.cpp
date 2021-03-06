#include "UI_Text.h"
#include "MemoryManager.h"


UI_Text::UI_Text(C_Canvas* owner, Rect rect) : UIElement(owner, UIElementType::TEXT, rect)
{

}

UI_Text::~UI_Text()
{

}

bool UI_Text::Update()
{
	bool ret = true;

	return ret;
}

bool UI_Text::CleanUp()
{
	bool ret = true;

	return ret;
}

std::string UI_Text::GetText() const
{
	return text;
}

uint UI_Text::GetFontSize() const
{
	return fontSize;
}


void UI_Text::SetText(const std::string& text)
{
	this->text = text;
}

void UI_Text::SetFontSize(const uint size)
{
	fontSize = size;
}