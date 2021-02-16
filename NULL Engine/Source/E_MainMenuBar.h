#ifndef __E_MAIN_MENU_BAR_H__
#define __E_MAIN_MENU_BAR_H__

#include "EditorPanel.h"

class E_MainMenuBar : public EditorPanel
{
public:
	E_MainMenuBar();
	~E_MainMenuBar();

	bool Draw		(ImGuiIO& io) override;
	bool CleanUp	() override;

private:
	bool FileMainMenuItem			();
	bool EditMainMenuItem			();
	bool WindowMainMenuItem			();
	bool ViewMainMenuItem			();
	bool GameObjectsMainMenuItem	();
	bool HelpMainMenuItem			();

	bool CloseAppPopup				();
};

#endif // !__E_MAIN_MENU_BAR_H__