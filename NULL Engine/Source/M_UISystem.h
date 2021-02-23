#ifndef __M_UISYSTEM_H__
#define __M_UISYSTEM_H__

#include "Module.h"

class ParsonNode;
typedef unsigned int uint;

class GameObject;
class C_Canvas;

class M_UISystem : public Module
{
public:

	M_UISystem(bool isActive = true);
	~M_UISystem();

	bool			Init(ParsonNode& config) override;
	UpdateStatus	PreUpdate(float dt) override;
	UpdateStatus	Update(float dt) override;
	UpdateStatus	PostUpdate(float dt) override;
	bool			CleanUp() override;

	bool			LoadConfiguration(ParsonNode& root) override;
	bool			SaveConfiguration(ParsonNode& root) const override;

public:
	

private:
	

};

#endif // !__M_UISYSTEM_H__