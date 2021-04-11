#include "Profiler.h"
#include "E_Navigation.h"

E_Navigation::E_Navigation() : EditorPanel("Navigation")
{

}

E_Navigation::~E_Navigation()
{

}

bool E_Navigation::Draw(ImGuiIO& io)
{
	OPTICK_CATEGORY("E_Hierarchy Draw", Optick::Category::Editor)

	ImGui::Begin(GetName());



	ImGui::End();

	return true;
}

bool E_Navigation::CleanUp()
{
	return true;
}