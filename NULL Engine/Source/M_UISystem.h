#ifndef __M_UISYSTEM_H__
#define __M_UISYSTEM_H__

#include "Module.h"

#include "Dependencies/FreeType/include/freetype/freetype.h"

class ParsonNode;
typedef unsigned int uint;

class GameObject;
class C_UI_Button;

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

public:

	FT_Library  library;
	FT_Face     standardFace;
	FT_UInt		glyphIndex;
	FT_ULong	charcode;
	FT_Int32	loadFlags = FT_LOAD_DEFAULT;

	std::vector<C_UI_Button*> activeButtons;
	C_UI_Button* hoveredButton = nullptr;

	bool isPressed = false;

private:
	C_UI_Button* buttonIterator = nullptr;
};

#endif // !__M_UISYSTEM_H__