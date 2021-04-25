#pragma once
#include <string>
#include "Macros.h"
namespace CoreCrossDllHelpers 
{
	NULL_API void CoreReleaseBuffer(char** buffer);
	NULL_API void CoreReleaseString(std::string& string);
}