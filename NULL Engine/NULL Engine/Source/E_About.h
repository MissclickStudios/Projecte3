#ifndef __E_ABOUT_H__
#define __E_ABOUT_H__

#include "EditorPanel.h"

class E_About : public EditorPanel
{
public:
	E_About();
	~E_About();

	bool Draw(ImGuiIO& io) override;

	bool CleanUp() override;

private:
	int a;
};

#endif // !__E_ABOUT_H__