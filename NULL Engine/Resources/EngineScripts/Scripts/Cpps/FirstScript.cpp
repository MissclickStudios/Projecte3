#include "..\..\..\Assets\Scripts\FirstScript.h"
#include "..\..\..\..\Source\Log.h"

FirstScript::FirstScript() : Script()
{
}

FirstScript::~FirstScript()
{
}

void FirstScript::Update()
{
	LOG("Hi");
}
