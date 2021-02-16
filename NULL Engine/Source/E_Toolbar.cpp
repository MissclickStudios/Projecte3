#include "Time.h"

#include "Application.h"
#include "M_Editor.h"

#include "E_Toolbar.h"

E_Toolbar::E_Toolbar() : EditorPanel("Toolbar"), played_once(false)
{

}

E_Toolbar::~E_Toolbar()
{

}

bool E_Toolbar::Draw(ImGuiIO& io)
{
	bool ret = true;

	ImGui::Begin("Toolbar");

	PlayAndStopButtons();

	ImGui::SameLine();

	PauseAndStepButtons();

	ImGui::SameLine();

	TimeScaleSlider();

	ImGui::SameLine();

	TimeDisplays();

	ImGui::End();

	return ret;
}

bool E_Toolbar::CleanUp()
{
	bool ret = true;



	return ret;
}

void E_Toolbar::PlayAndStopButtons()
{
	if (ImGui::Button("Play"))
	{
		App->editor->SaveSceneThroughEditor("PlayAutosave");

		Time::Game::Play();

		played_once = true;																			// Quickfix to avoid having conflicts between stop and pause.
		App->play	= true;
		App->pause	= false;
	}

	ImGui::SameLine();

	if (ImGui::Button("Stop"))
	{
		if (!played_once)
		{
			LOG("[ERROR] Play&Stop: Cannot Stop something that has not started yet!");
			return;
		}
		
		App->editor->LoadFileThroughEditor("Assets/Scenes/PlayAutosave.json");

		Time::Game::Stop();

		App->play	= false;
		App->pause	= false;
	}
}

void E_Toolbar::PauseAndStepButtons()
{
	if (ImGui::Button("Pause"))
	{
		Time::Game::Pause();

		App->pause	= true;
		App->play	= false;
	}

	ImGui::SameLine();

	if (ImGui::Button("Step"))
	{
		Time::Game::Step();

		App->step = true;
	}
}

void E_Toolbar::TimeScaleSlider()
{
	ImGui::SetNextItemWidth(75.0f);

	float time_scale = Time::Game::GetTimeScale();
	if (ImGui::SliderFloat("##", &time_scale, 0.250f, 4.000f, "X %.3f", ImGuiSliderFlags_None))
	{
		Time::Game::SetTimeScale(time_scale);
	}
}

void E_Toolbar::TimeDisplays()
{	
	ImGui::Text("Real Time: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", Time::Real::GetClock().GetTimeAsString().c_str());
	ImGui::SameLine();
	ImGui::Text("Game Time: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", Time::Game::GetClock().GetTimeAsString().c_str());
}