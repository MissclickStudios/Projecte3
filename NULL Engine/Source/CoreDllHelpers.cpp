#include "CoreDllHelpers.h"

void CoreCrossDllHelpers::CoreReleaseBuffer(char** buffer)
{
    RELEASE_ARRAY(*buffer);
}
