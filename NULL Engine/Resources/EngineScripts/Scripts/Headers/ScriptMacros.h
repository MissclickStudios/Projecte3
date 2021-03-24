#pragma once
#ifdef SCRIPTS_BUILD_DLL
#define SCRIPTS_API __declspec(dllexport)
#else
#define SCRIPTS_API __declspec(dllimport)
#endif

#define SCRIPTS_FUNCTION extern "C" SCRIPTS_API