#ifndef __E_IMGUI_DEMO_H__
#define __E_IMGUI_DEMO_H__

#include "EditorPanel.h"

class E_ImGuiDemo : public EditorPanel
{
public:
	E_ImGuiDemo();
	~E_ImGuiDemo();

	bool Draw(ImGuiIO& io) override;

	bool CleanUp() override;

private:
	bool			show_demo_window;
	bool			show_another_window;

	int				current_style;
	const char*		styles;

	float			f;
	int				counter;
};

#endif // !_E_IMGUI_DEMO_H__