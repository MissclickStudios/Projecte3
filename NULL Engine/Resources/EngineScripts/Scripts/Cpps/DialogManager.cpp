
#include "DialogManager.h"


DialogManager::DialogManager()
{

}

DialogManager::~DialogManager()
{

}

void DialogManager::Start()
{

}

void DialogManager::Update()
{


	switch (state)
	{
		case DialogState::NO_DIALOG:

			break;
		case DialogState::SLIDE_IN:

			break; 
		case DialogState::TALKING:

				break;
		case DialogState::TALKED:

			break;
	}
}

void DialogManager::CleanUp()
{

}

DialogSystem DialogManager::LoadDialogSystem(const char* path)
{

}

void DialogManager::StartDialog(const char* dialogName)
{
	state = DialogState::SLIDE_IN;
}

void DialogManager::StartDialog(DialogSystem* dialogSystem)
{
	state = DialogState::SLIDE_IN;
}

SCRIPTS_FUNCTION DialogManager* CreateCameraMovement()
{

}