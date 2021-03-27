#ifndef __GLOBALS_H__
#define __GLOBALS_H__

// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
#pragma warning( disable : 4530 ) // Warning that exceptions are disabled

#include <windows.h>
#include <stdio.h>

#include "ConfigurationSettings.h"
#include "FileSystemDefinitions.h"
#include "VariableDefinitions.h"
#include "VariableTypedefs.h"
#include "UpdateStatus.h"
#include "Macros.h"
#include "Log.h"

// Defining NULL just in case
#ifdef NULL
#undef NULL
#endif

#define NULL 0

#endif // !__GLOBALS_H__