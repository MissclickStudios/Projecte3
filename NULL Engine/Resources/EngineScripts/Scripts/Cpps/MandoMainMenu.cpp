
#include "GameObject.h"
#include "C_Animator.h"

#include "MandoMainMenu.h"

MandoMainMenu::MandoMainMenu()
{

}

MandoMainMenu::~MandoMainMenu()
{

}

void MandoMainMenu::Start()
{

	mandoAnimator = gameObject->GetComponent<C_Animator>();
}

void MandoMainMenu::Update()
{

	if (mandoAnimator != nullptr)
	{
		mandoAnimator->PlayClip("Preview", "Idle",0.f);
	}
}

void MandoMainMenu::CleanUp()
{

}

SCRIPTS_FUNCTION MandoMainMenu* CreateMandoMainMenu() 
{
	MandoMainMenu* script = new MandoMainMenu();

	return script;
}