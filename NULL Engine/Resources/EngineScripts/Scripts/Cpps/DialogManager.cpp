#include "GameObject.h"

#include "C_Canvas.h"
#include "R_Shader.h"
#include "C_UI_Image.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"

#include "DialogManager.h"

#include "Random.h"
#include "Log.h"
#include "MC_Time.h"
#include "JSONParser.h"
#include "CoreDllHelpers.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_FileSystem.h"
#include "M_Input.h"
#include "M_UISystem.h"

DialogManager::DialogManager()
{

}

DialogManager::~DialogManager()
{

}

void DialogManager::Start()
{
	GameObject* tmp = nullptr;

	tmp = App->scene->GetGameObjectByName(dialogCanvasName.c_str());
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

			App->uiSystem->PushCanvas(dialogCanvas);
			StartTalking();

			break; 
		case DialogState::TALKING:
			//Update text until it's full

			if (dialogButton->GetState() == UIButtonState::PRESSEDIN)
			{
				dialogText->SetText(currentLine->lineText.c_str());
				state = DialogState::TALKED;
				currentLineLetter = 0;
			}

			nextLetterTimer += MC_Time::Game::GetDT();

			if (nextLetterTimer > textSpeed)
			{
				std::string tmp = dialogText->GetText();
				tmp.push_back(currentLine->lineText[currentLineLetter]);
				dialogText->SetText(tmp.c_str());
				currentLineLetter++;
				nextLetterTimer = 0;

				if (currentLineLetter == currentLine->lineText.size() + 1)
				{
					state = DialogState::TALKED;
					currentLineLetter = 0;
				}
			}

				break;
		case DialogState::TALKED:
			//Wait for input, then go to next phrase if there is any. Close dialog if not

			if (dialogButton->GetState() == UIButtonState::PRESSEDIN)
			{
				//next line and go to talking or no dialog

					if (++currentLineIterator != currentDialog->lines.end()) //If it's not the last one
					{
						currentLine = (*currentLineIterator);
						StartNewLine();
						state = DialogState::TALKING;
					}
					else
					{
						App->uiSystem->RemoveActiveCanvas(dialogCanvas);
						state = DialogState::NO_DIALOG;
						//Clear elements
					}
			}

			break;
	}
}

void DialogManager::CleanUp()
{

}

void DialogManager::StartTalking()
{
	state = DialogState::TALKING;

	StartNewLine();
}

void DialogManager::StartNewLine()
{
	speakerText->SetText(currentLine->speakerName.c_str());

	//set speaker image

	dialogText->SetText("");
	currentLineLetter = 0;
}

DialogSystem* DialogManager::LoadDialogSystem(const char* path)
{
	char* buffer = nullptr;
	App->fileSystem->Load(path, &buffer);

	ParsonNode dialogRoot(buffer);

	CoreCrossDllHelpers::CoreReleaseBuffer(&buffer);

	//Fill Dialog System
	std::string dialogSystemName = dialogRoot.GetString("DialogSystemName");

	if (strcmp(dialogSystemName.c_str(), "[NOT FOUND]") == 0)
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

		if (strcmp(dialogName.c_str(), "[NOT FOUND]") == 0)
		{
			LOG("DialogPool node lacks name");
			continue;
		}

		Dialog* newDialog = new Dialog(dialogName.c_str());

		ParsonArray dialogArray = dialogNode.GetArray("Lines");

		for (uint j = 0; j < dialogArray.size; j++)
		{
			ParsonNode phraseNode = dialogArray.GetNode(j);
			newDialog->lines.push_back(new DialogLine(phraseNode.GetString("Text"), phraseNode.GetString("Speaker")));
		}

		newDialogSystem->dialogPool.push_back(newDialog);
	}

	dialogSystemsLoaded.push_back(newDialogSystem);

	return newDialogSystem;
}

bool DialogManager::StartDialog(const char* dialogName)
{
	currentDialogSystem = LoadDialogSystem(dialogName);

	if (currentDialogSystem == nullptr)
		return false;

	currentDialog = currentDialogSystem->dialogPool[Random::LCG::GetBoundedRandomUint(0, currentDialogSystem->dialogPool.size()-1)];
	currentLine = currentDialog->lines.front();
	currentLineIterator = currentDialog->lines.begin();

	state = DialogState::SLIDE_IN;

	//for (auto dialog = dialogSystemsLoaded.begin(); dialog != dialogSystemsLoaded.end(); ++dialog)
	//{
	//	if (strcmp((*dialog)->dialogSystemName.c_str(),dialogName) == 0) 
	//	{
	//		currentDialogSystem = (*dialog);
	//		currentDialog = (*dialog)->dialogPool[Random::LCG::GetBoundedRandomUint(0, (*dialog)->dialogPool.size())];
	//		currentLine = currentDialog->lines.front();
	//	}
	//}

	return true;
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
	INSPECTOR_STRING(script->dialogButtonName);
	INSPECTOR_STRING(script->speakerImageName);
	INSPECTOR_STRING(script->speakerTextName);
	INSPECTOR_STRING(script->textBackgroundName);
	return script;
}