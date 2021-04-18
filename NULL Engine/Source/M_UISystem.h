#ifndef __M_UISYSTEM_H__
#define __M_UISYSTEM_H__

#include "Module.h"
#include "C_Canvas.h"


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

	void UpdateInputCanvas();	
	void DeleteCanvas(C_Canvas* canvas);
	void DeleteActiveButton(C_UI_Button* button);

	void InitHoveredDecorations();
	void UpdateHoveredDecorations();
	void AddNewCanvas(C_Canvas* canvas);

public:

	//FT_Library  library;
	//FT_Face     standardFace;
	//FT_UInt		glyphIndex;
	//FT_ULong	charcode;
	//FT_Int32	loadFlags = FT_LOAD_DEFAULT;

	std::vector<C_Canvas*> canvasList;

	C_Canvas* inputCanvas = nullptr;

	bool isHoverDecorationAdded = false;

	C_UI_Image* hoveredDecorationL = nullptr;
	C_UI_Image* hoveredDecorationR = nullptr;

private:
};

#endif // !__M_UISYSTEM_H__