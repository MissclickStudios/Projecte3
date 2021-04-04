#pragma once
#include "C_Script.h"

#ifdef SCRIPTS_BUILD_DLL
#define SCRIPTS_API __declspec(dllexport)
#else
#define SCRIPTS_API __declspec(dllimport)
#endif

#define SCRIPTS_FUNCTION extern "C" SCRIPTS_API

#define INSPECTOR_INPUT_INT(INT_) C_Script::InspectorInputInt(&INT_, #INT_)