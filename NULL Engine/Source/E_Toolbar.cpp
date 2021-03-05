#include "Time.h"

#include "EngineApplication.h"
#include "M_Editor.h"

#include "E_Toolbar.h"

#include "MemoryManager.h"

E_Toolbar::E_Toolbar() : EditorPanel("Toolbar")
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
	if (!EngineApp->play)
	{
		if (ImGui::Button("Play"))
		{
			EngineApp->editor->SaveSceneThroughEditor("PlayAutosave");

			Time::Game::Play();

			EngineApp->play = true;
			EngineApp->pause = false;
		}
	}
	else
	{
		if (ImGui::Button("Stop"))
		{
			EngineApp->editor->LoadFileThroughEditor("Assets/Scenes/PlayAutosave.json");

			Time::Game::Stop();

			EngineApp->play = false;
			EngineApp->pause = false;
		}
	}
}

void E_Toolbar::PauseAndStepButtons()
{
	if (!EngineApp->pause)
	{
		if (ImGui::Button("Pause"))
		{
			if (EngineApp->play)
			{
				Time::Game::Pause();

				EngineApp->pause = true;
			}
		}
	}
	else
	{
		if (ImGui::Button("Resume"))
		{
			Time::Game::Play();

			EngineApp->pause = false;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Step"))
	{
		Time::Game::Step();

		EngineApp->step = true;
	}
}

void E_Toolbar::TimeScaleSlider()
{
	ImGui::SetNextItemWidth(75.0f);

	float timeScale = Time::Game::GetTimeScale();
	if (ImGui::SliderFloat("##", &timeScale, 0.250f, 4.000f, "X %.3f", ImGuiSliderFlags_None))
	{
		Time::Game::SetTimeScale(timeScale);
	}
}

void E_Toolbar::TimeDisplays()
{	
	//TODO: return string from dll memo leak
	//can't call Time::Real::GetClock().GetTimeAsString() ???
	Hourglass realTime = Time::Real::GetClock();
	Hourglass gameTime = Time::Game::GetClock();
	std::string hi = gameTime.GetTimeAsString();
	LOG("%d", sizeof(std::string));
	ImGui::Text("Real Time: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", std::string(std::to_string(realTime.hours) + "h " + std::to_string(realTime.minutes) + "m " + std::to_string(realTime.seconds) + "s").c_str());
	ImGui::SameLine();
	ImGui::Text("Game Time: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", std::string(std::to_string(gameTime.hours) + "h " + std::to_string(gameTime.minutes) + "m " + std::to_string(gameTime.seconds) + "s").c_str());
}