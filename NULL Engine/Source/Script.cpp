#include "Script.h"

bool Script::IsScriptEnabled() const
{
	return (enabled == nullptr) ? false : *enabled;
}

void Script::SetScriptEnable(const bool& enable)
{
	if (enable != *enabled) {
		*enabled = enable;

		if (enable) 
			OnEnable();

		else
			OnDisable();
		
	}
}
