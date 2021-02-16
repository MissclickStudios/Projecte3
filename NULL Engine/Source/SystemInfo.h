#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__

#include "HardwareInfo.h"
#include "SoftwareInfo.h"

struct SystemInfo
{
	SystemInfo();

	HardwareInfo hardware;
	SoftwareInfo software;
};

#endif // !__SYSTEM_INFO_H__