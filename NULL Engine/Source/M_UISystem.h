#ifndef __M_UISYSTEM_H__
#define __M_UISYSTEM_H__

#include "Module.h"
#include "C_Canvas.h"


class ParsonNode;
typedef unsigned int uint;

class GameObject;
class C_UI_Button;
class C_UI_Image;
class R_Texture;

class MISSCLICK_API M_UISystem : public Module
{
public:

	M_UISystem(bool isActive = true);
	~M_UISystem();

	bool Init(ParsonNode& config) override;
	bool Start() override;
	UpdateStatus PreUpdate(float dt) override;
	UpdateStatus Update(float dt) override;
	UpdateStatus PostUpdate(float dt) override;
	bool CleanUp() override;

	void CleanUpScene();
		 
	bool LoadConfiguration(ParsonNode& root) override;
	bool SaveConfiguration(ParsonNode& root) const override;

	void UpdateInputCanvas();
	void AddNewCanvas(C_Canvas* canvas);
	void DeleteCanvas(C_Canvas* canvas);

	void UpdateCanvasList();

	void DeleteActiveButton(C_UI_Button* button);

	void InitHoveredDecorations();
	void UpdateHoveredDecorations();

public:

	R_Texture* buttonHoverDecor = nullptr;

	std::vector<C_Canvas*> canvasList;

	C_Canvas* inputCanvas = nullptr;

	bool isHoverDecorationAdded = false;

	C_UI_Image* hoveredDecorationL = nullptr;
	C_UI_Image* hoveredDecorationR = nullptr;

	uint priorityIterator = 0;

private:
	C_Canvas* canvasIterator = nullptr;
};

#endif // !__M_UISYSTEM_H__