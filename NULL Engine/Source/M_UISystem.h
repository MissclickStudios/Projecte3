#ifndef __M_UISYSTEM_H__
#define __M_UISYSTEM_H__

#include "Module.h"
#include "C_Canvas.h"

#include "Dependencies/FreeType/include/freetype/freetype.h"

class ParsonNode;
typedef unsigned int uint;

class GameObject;
class C_UI_Button;
class C_UI_Image;

class NULL_API M_UISystem : public Module
{
public:

	M_UISystem(bool isActive = true);
	~M_UISystem();

	bool Init(ParsonNode& config) override;
	UpdateStatus PreUpdate(float dt) override;
	UpdateStatus Update(float dt) override;
	UpdateStatus PostUpdate(float dt) override;
	bool CleanUp() override;
		 
	bool LoadConfiguration(ParsonNode& root) override;
	bool SaveConfiguration(ParsonNode& root) const override;

	bool CheckButtonStates(); // Returns false if no buttons are hovered/pressed

	void UpdateActiveButtons();
	void InitHoveredDecorations();
	void UpdateHoveredDecorations();
	void DeleteActiveButton(C_UI_Button* button);

public:

	FT_Library  library;
	FT_Face     standardFace;
	FT_UInt		glyphIndex;
	FT_ULong	charcode;
	FT_Int32	loadFlags = FT_LOAD_DEFAULT;

	std::vector<C_UI_Button*> activeButtons;
	C_UI_Button* hoveredButton = nullptr;

	bool isMainMenu = true;
	bool isHoverDecorationAdded = false;
	C_UI_Image* hoveredDecorationL = nullptr;
	C_UI_Image* hoveredDecorationR = nullptr;

private:
	C_UI_Button* buttonIterator = nullptr;
};

#endif // !__M_UISYSTEM_H__