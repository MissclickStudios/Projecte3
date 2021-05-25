#ifndef __M_UISYSTEM_H__
#define __M_UISYSTEM_H__

#include <list>
#include <vector>
#include "Module.h"


class ParsonNode;
typedef unsigned int uint;

class GameObject;
class C_Canvas;
class R_Texture;

class MISSCLICK_API M_UISystem : public Module
{
public:

	M_UISystem(bool isActive = true);
	~M_UISystem();
	UpdateStatus Update(float dt) override;
	bool CleanUp() override;
		 
	bool LoadConfiguration(ParsonNode& root) override;
	bool SaveConfiguration(ParsonNode& root) const override;

	void PushCanvas(C_Canvas* canvas);
	C_Canvas* PopCanvas();
	void RemoveActiveCanvas(C_Canvas* canvas);

	const std::list<C_Canvas*>& GetActiveCanvas()const;
	const std::vector<C_Canvas*>& GetAllCanvas()const;
	void ClearActiveCanvas();
	void ReorderCanvasChildren();
	void SaveCanvasChildrenOrder();

private:
	std::list<C_Canvas*> activeCanvas;
	std::vector<C_Canvas*> allCanvas;

	friend class C_Canvas;
};

#endif // !__M_UISYSTEM_H__