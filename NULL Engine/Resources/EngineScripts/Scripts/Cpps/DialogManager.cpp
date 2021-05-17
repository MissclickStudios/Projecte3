#include "GameObject.h"

#include "C_Canvas.h"
#include "R_Shader.h"
#include "C_UI_Image.h"
#include "C_UI_Text.h"

#include "DialogManager.h"

#include "Log.h"
#include "MC_Time.h"
#include "JSONParser.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_FileSystem.h"

DialogManager::DialogManager()
{

}

DialogManager::~DialogManager()
{

}

void DialogManager::Start()
{
	GameObject* tmp = nullptr;

	tmp = App->scene->GetGameObjectByName(speakerImageName.c_str());
	if (tmp != nullptr)
		dialogCanvas = tmp->GetComponent<C_Canvas>();

	tmp = App->scene->GetGameObjectByName(speakerImageName.c_str());
	if (tmp != nullptr)
		speakerImage = tmp->GetComponent<C_UI_Image>();

	tmp = App->scene->GetGameObjectByName(textBackgroundName.c_str());
	if (tmp != nullptr)
		textBackground = tmp->GetComponent<C_UI_Image>();

	tmp = App->scene->GetGameObjectByName(speakerTextName.c_str());
	if (tmp != nullptr)
		speakerText = tmp->GetComponent<C_UI_Text>();

	tmp = App->scene->GetGameObjectByName(dialogTextName.c_str());
	if (tmp != nullptr)
		dialogText = tmp->GetComponent<C_UI_Text>();

}

void DialogManager::Update()
{

	switch (state)
	{
		case DialogState::NO_DIALOG:

			break;
		case DialogState::SLIDE_IN:

			StartTalking();
			break; 
		case DialogState::TALKING:
			//Update text until it's full

			nextLetterTimer += MC_Time::Game::GetDT();

			if (nextLetterTimer > (nextLetterBaseTime * textSpeed))
			{

			}

				break;
		case DialogState::TALKED:
			//ait for input, then go to next phrase if there is any. Close dialog if not


			break;
	}
}

void DialogManager::CleanUp()
{

}

void DialogManager::StartTalking()
{
	state = DialogState::TALKING;


}

DialogSystem* DialogManager::LoadDialogSystem(const char* path)
{
	char* buffer = nullptr;
	App->fileSystem->Load(path, &buffer);

	ParsonNode dialogRoot(buffer);

	RELEASE_ARRAY(buffer);

	//Fill Dialog System
	std::string dialogSystemName = dialogRoot.GetString("DialogSystemName");

	if (strcmp(dialogSystemName.c_str(), "[NOT FOUND]") != 0)
	{
		LOG("DialogSystem Lacks name");
		return nullptr;
	}
	
	DialogSystem* newDialogSystem = new DialogSystem(dialogSystemName.c_str());

	ParsonArray dialogPoolArray = dialogRoot.GetArray("DialogPool");

	for (uint i = 0; i < dialogPoolArray.size; i++)
	{
		ParsonNode dialogNode = dialogPoolArray.GetNode(i);

		std::string dialogName = dialogNode.GetString("DialogName");

		if (strcmp(dialogName.c_str(), "[NOT FOUND]") != 0)
		{
			LOG("DialogPool node lacks name");
			continue;
		}

		Dialog* newDialog = new Dialog(dialogName.c_str());

		ParsonArray dialogArray = dialogNode.GetArray("Lines");

		for (uint j = 0; j < dialogArray.size; j++)
		{
			ParsonNode phraseNode = dialogArray.GetNode(j);
			newDialog->phrases.push_back(new DialogLine(phraseNode.GetString("Text"), phraseNode.GetString("Speaker")));
		}

		newDialogSystem->dialogPool.push_back(newDialog);
	}

	dialogSystemsLoaded.push_back(newDialogSystem);

	return newDialogSystem;
}

void DialogManager::StartDialog(const char* dialogName)
{
	state = DialogState::SLIDE_IN;

	for (auto dialogs = dialogSystemsLoaded.begin(); dialogs != dialogSystemsLoaded.end(); ++dialogs)
	{
		if (strcmp((*dialogs)->dialogSystemName.c_str(),dialogName) == 0)
		{

		}
	}
}

void DialogManager::StartDialog(DialogSystem* dialogSystem)
{
	state = DialogState::SLIDE_IN;

	currentDialogSystem = dialogSystem;
}

DialogManager* CreateDialogManager()
{
	DialogManager* script = new DialogManager();
	INSPECTOR_STRING(script->dialogCanvasName);
	INSPECTOR_STRING(script->dialogTextName);
	INSPECTOR_STRING(script->speakerImageName);
	INSPECTOR_STRING(script->speakerTextName);
	INSPECTOR_STRING(script->textBackgroundName);
	return script;
}