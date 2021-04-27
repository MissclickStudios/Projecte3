#pragma once
#include <string>
#include "Macros.h"
namespace CoreCrossDllHelpers 
{
	MISSCLICK_API void CoreReleaseBuffer(char** buffer);
	MISSCLICK_API void CoreReleaseString(std::string& string);
}