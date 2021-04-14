#ifndef __E_NAVIGATION_H__
#define __E_NAVIGATION_H__

#include "EditorPanel.h"

class E_Navigation : public EditorPanel
{
public:
	E_Navigation();
	~E_Navigation();

	bool Draw(ImGuiIO& io) override;
	bool CleanUp() override;

private:

};

#endif // !__E_NAVIGATION_H__