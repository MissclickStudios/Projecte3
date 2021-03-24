#ifndef __M_GAMEMANAGER_H__
#define __M_GAMEMANAGER_H__

#include "Module.h"

class M_GameManager : public Module
{
public:

	M_GameManager(bool isActive = true);
	~M_GameManager();

	bool Init(ParsonNode& config) override;
	bool Start()override;
	/*UpdateStatus	PreUpdate(float dt) override;
	UpdateStatus	Update(float dt) override;
	UpdateStatus	PostUpdate(float dt) override;
	bool			CleanUp() override;

	bool			LoadConfiguration(ParsonNode& root) override;
	bool			SaveConfiguration(ParsonNode& root) const override;*/

	void PostSceneRendering() override;

};

#endif // !__M_GAMEMANAGER_H__