// -------------------------------------------------------
// E_About.cpp
// Will always be inactive and will be drawn by E_Toolbar
// -------------------------------------------------------

#include "Application.h"
#include "M_Editor.h"

#include "E_About.h"

E_About::E_About() : EditorPanel("About", false)
{

}

E_About::~E_About()
{

}

bool E_About::Draw(ImGuiIO& io)
{
	bool ret = true;

	ImGui::OpenPopup("About");

	SetIsHovered();
	
	if (ImGui::BeginPopupModal("About", &App->editor->show_about_popup))
	{	
		ImGui::Text("NULL Engine v0.75 - By Angel Gonzalez (CITM UPC)");
		ImGui::Text("This engine has been coded from scratch in C++ with the help of the libraries shown below.");

		ImGui::Text("");
		ImGui::Separator();

		ImGui::Text("Libraries:");
		ImGui::Text("OpenGL v4.6 ------------------------>");		ImGui::SameLine();		if (ImGui::Button("Get OpenGL v4.6"))			{ App->RequestBrowser("https://www.opengl.org/sdk/"); }
		ImGui::Text("Glew v2.1.0 ------------------------>");		ImGui::SameLine();		if (ImGui::Button("Get Glew v2.1.0"))			{ App->RequestBrowser("http://glew.sourceforge.net/index.html"); }
		ImGui::Text("ImGui v1.79 (Docking Branch) ------->");		ImGui::SameLine();		if (ImGui::Button("Get ImGui v1.79 Docking"))	{ App->RequestBrowser("https://github.com/ocornut/imgui/tree/docking"); }
		ImGui::Text("ImGuizmo v1.61 --------------------->");		ImGui::SameLine();		if (ImGui::Button("Get ImGuizmo v1.61"))		{ App->RequestBrowser("https://github.com/CedricGuillemet/ImGuizmo"); }
		ImGui::Text("MathGeoLib v1.5 -------------------->");		ImGui::SameLine();		if (ImGui::Button("Get MathGeoLib v1.5"))		{ App->RequestBrowser("https://github.com/juj/MathGeoLib/releases/tag/v1.5"); }
		ImGui::Text("PhysFS v3.0.2 ---------------------->");		ImGui::SameLine();		if (ImGui::Button("Get PhysFS v3.0.2"))			{ App->RequestBrowser("https://icculus.org/physfs/"); }
		ImGui::Text("Parson v1.1.0 ---------------------->");		ImGui::SameLine();		if (ImGui::Button("Get Parson v1.1.0"))			{ App->RequestBrowser("https://github.com/kgabis/parson"); }
		ImGui::Text("Assimp v3.1.1 ---------------------->");		ImGui::SameLine();		if (ImGui::Button("Get Assimp v3.1.1"))			{ App->RequestBrowser("http://www.assimp.org/index.php/downloads"); }
		ImGui::Text("DevIL v1.8.0 ----------------------->");		ImGui::SameLine();		if (ImGui::Button("Get DevIL v1.8.0"))			{ App->RequestBrowser("http://openil.sourceforge.net/download.php"); }
		ImGui::Text("mmgr ------------------------------->");		ImGui::SameLine();		if (ImGui::Button("Get mmgr"))					{ App->RequestBrowser("http://www.paulnettle.com/pub/FluidStudios/MemoryManagers/Fluid_Studios_Memory_Manager.zip"); }

		ImGui::Text("");
		ImGui::Separator();

		ImGui::Text("LICENSE:");
		ImGui::Text("MIT License");
		ImGui::Text("");
		ImGui::Text("Copyright (c) 2021 [Angel Gonzalez]");
		ImGui::Text("");
		ImGui::Text("Permission is hereby granted, free of charge, to any person obtaining a copy ");
		ImGui::Text("of this software and associated documentation files (the 'Software'), to deal");
		ImGui::Text("in the Software without restriction, including without limitation the rights ");
		ImGui::Text("to use, copy, modify, merge, publish, distribute, sublicense, and/or sell    ");
		ImGui::Text("copies of the Software, and to permit persons to whom the Software is        ");
		ImGui::Text("furnished to do so, subject to the following conditions:");
		ImGui::Text("");
		ImGui::Text("The above copyright notice and this permission notice shall be included in all");
		ImGui::Text("copies or substantial portions of the Software.");
		ImGui::Text("");
		ImGui::Text("THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   ");
		ImGui::Text("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     ");
		ImGui::Text("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  ");
		ImGui::Text("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       ");
		ImGui::Text("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
		ImGui::Text("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN    ");
		ImGui::Text("THE SOFTWARE.");

		ImGui::EndPopup();
	}

	return ret;
}

bool E_About::CleanUp()
{
	bool ret = true;

	return ret;
}