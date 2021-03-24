#include "FirstScript.h"
#include "Log.h"

FirstScript::FirstScript() : Script()
{
}

FirstScript::~FirstScript()
{
}

void FirstScript::Awake()
{
	LOG("Awake");
}

void FirstScript::Start()
{
	LOG("Start");
}

void FirstScript::PreUpdate()
{
	LOG("PreUpdate");
}

void FirstScript::Update()
{
	LOG("Update: First variable = %d", firstVariable);
}

void FirstScript::PostUpdate()
{
	LOG("PostUpdate");
}

void FirstScript::CleanUp()
{
	LOG("CleanUp");
}
