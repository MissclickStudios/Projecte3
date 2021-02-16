#ifndef __E_TIMELINE_H__
#define __E_TIMELINE_H__

#include "EditorPanel.h"

class E_Timeline : public EditorPanel
{
public:
	E_Timeline();
	~E_Timeline();

	bool Draw		(ImGuiIO& io) override;
	bool CleanUp	() override;

private:
	float current_time;
};

#endif // !__E_TIMELINE_H__