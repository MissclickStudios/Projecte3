#ifndef _DEBUG
#define AK_OPTIMIZED
#else
#pragma comment(lib,"CommunicationCentral.lib")
#pragma comment(lib,"ws2_32.lib")
#endif

#include "M_Audio.h"

M_Audio::M_Audio(bool isActive) : Module("Audio", isActive)
{
}

M_Audio::~M_Audio()
{
}
