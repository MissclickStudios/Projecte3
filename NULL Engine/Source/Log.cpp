#include "Application.h"

#include "Log.h"
#include <windows.h>
#include <stdio.h>

void log(const char file[], int line, const char* format, ...)
{
	static char tmpString[4096];
	static char tmpString2[4096];
	static va_list  ap;

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmpString, 4096, format, ap);
	va_end(ap);
	sprintf_s(tmpString2, 4096, "\n%s(%d) : %s", file, line, tmpString);
	OutputDebugString(tmpString2);

	if (App != nullptr)
	{
		App->AddEditorLog(tmpString2);
	}
}