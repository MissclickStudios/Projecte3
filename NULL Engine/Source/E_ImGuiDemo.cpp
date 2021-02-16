#include "Application.h"
#include "M_Window.h"
#include "M_Editor.h"

#include "E_ImGuiDemo.h"

E_ImGuiDemo::E_ImGuiDemo() : EditorPanel("ImGuiDemo")
{
	show_demo_window		= true;
	show_another_window		= false;

	current_style			= (int)IMGUI_STYLE::DARK;

	f						= 0.0f;
	counter					= 0;
}

E_ImGuiDemo::~E_ImGuiDemo()
{

}

bool E_ImGuiDemo::Draw(ImGuiIO& io)
{
	bool ret = true;

	// Showing ImGui's predetermined demo window
	if (show_demo_window)
	{
		ImGui::ShowDemoWindow();
	}

	// Creating a simple window
	{
		ImGui::Begin("Hello world!");								// Will create a window with "Hello World!" as the title. Until ImGui::End() all elements will be appended to this window.

		SetIsHovered();
		
		if (ImGui::Button("SHOW ME THE DATA"))
		{
			//return UPDATE_STATUS::STOP;

			App->display_framerate_data = !App->display_framerate_data;
		}

		ImGui::SameLine();

		if (ImGui::Button("YEETUS THIS WINDOW"))
		{
			ret = false;
		}

		ImGui::CheckboxFlags("io.ConfigFlags: DockingEnable", (unsigned int*)&io.ConfigFlags, ImGuiConfigFlags_DockingEnable);

		ImGui::Text("This text has been brought to you by Euro Shave Club.");			// Will create a label. Can also use format strings.
		ImGui::Checkbox("ImGui Demo Window", &show_demo_window);						// Checkbox that will modify the bool that it gets passed as argument.
		ImGui::Checkbox("Sneaky Window", &show_another_window);

		const char* styles[] = { "Classic", "Light", "Dark", "TBD" };
		ImGui::Combo("ImGui Style", &current_style, styles, IM_ARRAYSIZE(styles));

		if (current_style == (int)IMGUI_STYLE::CLASSIC)
		{
			ImGui::StyleColorsClassic();
		}
		else if (current_style == (int)IMGUI_STYLE::LIGHT)
		{
			ImGui::StyleColorsLight();
		}
		else if (current_style == (int)IMGUI_STYLE::DARK)
		{
			ImGui::StyleColorsDark();
		}

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);									// Will create a slider that will be able to edit 1 variable from 0.0f to 1.0f.
		ImGui::ColorEdit3("clear color", (float*)&App->editor->clear_color);			// Will create 3 sliders in a row that will represent a colour (RGB).

		if (ImGui::Button("Button"))
		{
			++counter;
		}

		//ImGui::PlotHistogram();

		ImGui::SameLine();																// Specifies that the next element to be created will be created in the same row as the previous one.
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Separator();

		if (ImGui::CollapsingHeader("User Guide"))
		{
			ImGui::ShowUserGuide();
		}

		ImGui::End();
	}

	if (show_another_window)
	{
		ImGui::Begin("Sneaky Window", &show_another_window);
		ImGui::Text("Hello from sneaky window!");

		if (ImGui::Button("Close me"))
		{
			show_another_window = false;
		}


		ImGui::End();
	}

	return ret;
}

bool E_ImGuiDemo::CleanUp()
{
	bool ret = true;

	return ret;
}