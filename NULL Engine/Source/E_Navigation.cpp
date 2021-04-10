#include "E_Navigation.h"

E_Navigation::E_Navigation() : EditorPanel("Navigation")
{

}

E_Navigation::~E_Navigation()
{

}

bool E_Navigation::Draw(ImGuiIO& io)
{
	ImGui::Begin(GetName());



	ImGui::End();

	return true;
}

bool E_Navigation::CleanUp()
{
	return true;
}