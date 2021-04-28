#include "CoreDllHelpers.h"

void CoreCrossDllHelpers::CoreReleaseBuffer(char** buffer)
{
    RELEASE_ARRAY(*buffer);
}

void CoreCrossDllHelpers::CoreReleaseString(std::string& string)
{
    string.~basic_string();
}
