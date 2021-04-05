#pragma once
#include "C_Script.h"

#ifdef SCRIPTS_BUILD_DLL
#define SCRIPTS_API __declspec(dllexport)
#else
#define SCRIPTS_API __declspec(dllimport)
#endif

#define SCRIPTS_FUNCTION extern "C" SCRIPTS_API

#define INSPECTOR_INPUT_INT(INT_) C_Script::InspectorInputInt(&INT_, #INT_)
#define INSPECTOR_DRAGABLE_INT(INT_) C_Script::InspectorDragableInt(&INT_, #INT_)
#define INSPECTOR_SLIDER_INT(INT_, MIN_, MAX_) C_Script::InspectorSliderInt(&INT_, #INT_, MIN_, MAX_)

#define INSPECTOR_CHECKBOX_BOOL(BOOL_) C_Script::InspectorBool(&BOOL_, #BOOL_)

#define INSPECTOR_INPUT_FLOAT(FLOAT_) C_Script::InspectorInputFloat(&FLOAT_, #FLOAT_)
#define INSPECTOR_DRAGABLE_FLOAT(FLOAT_) C_Script::InspectorDragableFloat(&FLOAT_, #FLOAT_)
#define INSPECTOR_SLIDER_FLOAT(FLOAT_, MIN_, MAX_) C_Script::InspectorSliderFloat(&FLOAT_, #FLOAT_, MIN_, MAX_)