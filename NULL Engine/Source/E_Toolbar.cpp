#include "M_Editor.h"
#include "MC_Time.h"
#include "Color.h"

#include "EngineApplication.h"
#include "E_Toolbar.h"
#include "Profiler.h"
#include "M_ScriptManager.h"
#include "FileSystemDefinitions.h"
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
	OPTICK_CATEGORY("E_Toolbar Draw", Optick::Category::Editor)

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
			MC_Time::Game::Play();

			EngineApp->editor->SaveSceneThroughEditor(AUTOSAVE_FILE_NAME);

			EngineApp->scriptManager->InitScripts();

		}
	}
	else
	{
		if (ImGui::Button("Stop"))
		{
			EngineApp->gameState = GameState::STOP;
			MC_Time::Game::Stop();

			EngineApp->scriptManager->CleanUpScripts();

			EngineApp->editor->LoadFileThroughEditor("Assets/Scenes/PlayAutosave.json");

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
				MC_Time::Game::Pause();
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
			MC_Time::Game::Play();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Step"))
	{
		if (EngineApp->gameState == GameState::PAUSE)
		{
			EngineApp->gameState = GameState::STEP;
			MC_Time::Game::Step();
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

	float timeScale = MC_Time::Game::GetTimeScale();
	if (ImGui::SliderFloat("##", &timeScale, 0.250f, 4.000f, "X %.3f", ImGuiSliderFlags_None))
	{
		MC_Time::Game::SetTimeScale(timeScale);
	}
}

void E_Toolbar::TimeDisplay()
{	
	Hourglass realTime = MC_Time::Real::GetClock();
	Hourglass gameTime = MC_Time::Game::GetClock();

	ImGui::Text("Real Time:"); ImGui::SameLine(); ImGui::TextColored(&Yellow, "%uh %um %.3fs", realTime.hours, realTime.minutes, realTime.seconds);
	ImGui::SameLine();
	ImGui::Text("Game Time:"); ImGui::SameLine(); ImGui::TextColored(&Yellow, "%uh %um %.3fs", gameTime.hours, gameTime.minutes, gameTime.seconds);
}