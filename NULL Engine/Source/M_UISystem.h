#ifndef __M_UISYSTEM_H__
#define __M_UISYSTEM_H__

#include <list>
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
	//void DeleteCanvas(C_Canvas* canvas); TODO!!! Maybe handle these on the destructor of component canvas

	const std::list<C_Canvas*>& GetActiveCanvas()const;
	void ClearActiveCanvas();

private:
	std::list<C_Canvas*> activeCanvas;
};

#endif // !__M_UISYSTEM_H__