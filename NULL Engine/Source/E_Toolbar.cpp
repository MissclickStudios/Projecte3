#include "M_Editor.h"
#include "Time.h"
#include "Color.h"

#include "EngineApplication.h"
#include "E_Toolbar.h"
#include "Profiler.h"
#include "M_ScriptManager.h"
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

	TimeDisplay();

	ImGui::SameLine();

	TimeScaleSlider();

	ImGui::SameLine((ImGui::GetWindowWidth() * 0.4f));

	PlayAndStopButtons();

	ImGui::SameLine();

	PauseAndStepButtons();

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
	if (EngineApp->gameState != GameState::PLAY)
	{
		if (ImGui::Button("Play"))
		{
			EngineApp->gameState = GameState::PLAY;
			Time::Game::Play();

			EngineApp->scriptManager->InitScripts();

			EngineApp->editor->SaveSceneThroughEditor("PlayAutosave");
			EngineApp->play = true;
			EngineApp->pause = false;

		}
	}
	else
	{
		if (ImGui::Button("Stop"))
		{
			EngineApp->gameState = GameState::STOP;
			Time::Game::Stop();

			EngineApp->scriptManager->CleanUpScripts();

			EngineApp->editor->LoadFileThroughEditor("Assets/Scenes/PlayAutosave.json");
			EngineApp->play = false;
			EngineApp->pause = false;

		}
	}
}

void E_Toolbar::PauseAndStepButtons()
{
	if (EngineApp->gameState != GameState::PAUSE)
	{
		if (ImGui::Button("Pause"))
		{
			if (EngineApp->gameState == GameState::PLAY)
			{
				EngineApp->gameState = GameState::PAUSE;
				Time::Game::Pause();
			}
			else
			{
				LOG("[WARNING] Editor Toolbar: Cannot Pause if the Engine is not in Game Mode!");
			}
		}
	}
	else
	{
		if (ImGui::Button("Resume"))
		{
			EngineApp->gameState = GameState::PLAY;
			Time::Game::Play();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Step"))
	{
		if (EngineApp->gameState == GameState::PAUSE)
		{
			EngineApp->gameState = GameState::STEP;
			Time::Game::Step();
		}
		else
		{
			LOG("[WARNING] Editor Toolbar: Cannot Step if the Game is not Paused!");
		}
	}
}

void E_Toolbar::TimeScaleSlider()
{
	ImGui::SetNextItemWidth(75.0f);

	//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetWindowHeight() * 0.5f));									// ATTENTION: THIS AFFECTS THE POSITIONING OF OTHER ITEMS.

	float timeScale = Time::Game::GetTimeScale();
	if (ImGui::SliderFloat("##", &timeScale, 0.250f, 4.000f, "X %.3f", ImGuiSliderFlags_None))
	{
		Time::Game::SetTimeScale(timeScale);
	}
}

void E_Toolbar::TimeDisplay()
{	
	std::string realTimeString = "[NONE]";
	std::string gameTimeString = "[NONE]";


	GetTimeDisplayStrings(realTimeString, gameTimeString);

	//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetWindowHeight() * 0.575f));									// ATTENTION: THIS AFFECTS THE POSITIONING OF OTHER ITEMS.

	ImGui::Text("Real Time:"); ImGui::SameLine(); ImGui::TextColored(&Yellow, " %s", realTimeString.c_str());
	ImGui::SameLine();
	ImGui::Text("Game Time:"); ImGui::SameLine(); ImGui::TextColored(&Yellow, " %s", gameTimeString.c_str());
}

void E_Toolbar::GetTimeDisplayStrings(std::string& realTimeString, std::string& gameTimeString)
{
	//TODO: return string from dll memo leak
	//can't call Time::Real::GetClock().GetTimeAsString() ???
	Hourglass realTime = Time::Real::GetClock();
	Hourglass gameTime = Time::Game::GetClock();
	realTimeString = (std::to_string(realTime.hours) + "h " + std::to_string(realTime.minutes) + "m " + std::to_string(realTime.seconds) + "s");
	gameTimeString = (std::to_string(gameTime.hours) + "h " + std::to_string(gameTime.minutes) + "m " + std::to_string(gameTime.seconds) + "s");

}