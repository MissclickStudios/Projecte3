#include "ItemMenuManager.h"

#include "Application.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_UI_Button.h"
#include "C_UI_Text.h"

ItemMenuManager::ItemMenuManager()
{
}

ItemMenuManager::~ItemMenuManager()
{
}

void ItemMenuManager::Start()
{
	GameObject* gameObject = App->scene->GetGameObjectByName(buyButtonName.c_str());
	buy = gameObject->GetComponent<C_UI_Button>();

	gameObject = App->scene->GetGameObjectByName(nameTextName.c_str());
	name = gameObject->GetComponent<C_UI_Text>();

	gameObject = App->scene->GetGameObjectByName(descriptionTextName.c_str());
	description = gameObject->GetComponent<C_UI_Text>();

	gameObject = App->scene->GetGameObjectByName(priceTextName.c_str());
	price = gameObject->GetComponent<C_UI_Text>();

}

void ItemMenuManager::Update()
{
}
