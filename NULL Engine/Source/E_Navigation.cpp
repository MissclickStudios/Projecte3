#include "Profiler.h"
#include "E_Navigation.h"

#include "Application.h"
#include "M_Detour.h"

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

	if (ImGui::Button("Bake"))
	{
		App->detour->BakeNavMesh();
	}

	ImGui::End();

	return true;
}

bool E_Navigation::CleanUp()
{
	return true;
}