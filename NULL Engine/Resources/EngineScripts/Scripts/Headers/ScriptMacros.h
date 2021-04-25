#pragma once
#include "C_Script.h"

#ifdef SCRIPTS_BUILD_DLL
#define SCRIPTS_API __declspec(dllexport)
#else
#define SCRIPTS_API __declspec(dllimport)
#endif

#define SCRIPTS_FUNCTION extern "C" SCRIPTS_API
#define ALLOWED_INHERITANCE
#define ENGINE_ENUM

#define INSPECTOR_INPUT_INT(INT_) C_Script::InspectorInputInt(&INT_, #INT_)
#define INSPECTOR_DRAGABLE_INT(INT_) C_Script::InspectorDragableInt(&INT_, #INT_)
#define INSPECTOR_SLIDER_INT(INT_, MIN_, MAX_) C_Script::InspectorSliderInt(&INT_, #INT_, MIN_, MAX_)

#define INSPECTOR_CHECKBOX_BOOL(BOOL_) C_Script::InspectorBool(&BOOL_, #BOOL_)

#define INSPECTOR_INPUT_FLOAT(FLOAT_) C_Script::InspectorInputFloat(&FLOAT_, #FLOAT_)
#define INSPECTOR_DRAGABLE_FLOAT(FLOAT_) C_Script::InspectorDragableFloat(&FLOAT_, #FLOAT_)
#define INSPECTOR_SLIDER_FLOAT(FLOAT_, MIN_, MAX_) C_Script::InspectorSliderFloat(&FLOAT_, #FLOAT_, MIN_, MAX_)

#define INSPECTOR_INPUT_FLOAT3(FLOAT3_) C_Script::InspectorInputFloat3(&FLOAT3_, #FLOAT3_)
#define INSPECTOR_DRAGABLE_FLOAT3(FLOAT3_) C_Script::InspectorDragableFloat3(&FLOAT3_, #FLOAT3_)
#define INSPECTOR_SLIDER_FLOAT3(FLOAT3_, MIN_, MAX_) C_Script::InspectorSliderFloat3(&FLOAT3_, #FLOAT3_, MIN_, MAX_)

#define INSPECTOR_STRING(STRING_) C_Script::InspectorString(&STRING_, #STRING_)
#define INSPECTOR_TEXT(STRING_) C_Script::InspectorText(&STRING_, #STRING_)

#define INSPECTOR_PREFAB(PREFAB_) C_Script::InspectorPrefab(&PREFAB_, #PREFAB_)

#define INSPECTOR_GAMEOBJECT(GAMEOBJECT_) C_Script::InspectorGameObject(&GAMEOBJECT_, #GAMEOBJECT_)

#define INSPECTOR_VECTOR_STRING(VECTORSTRING_) C_Script::InspectorStringVector(&VECTORSTRING_, #VECTORSTRING_)

#define INSPECTOR_ENUM(ENUM_, ENUMNAME_, DEFINITION_FILE_) C_Script::InspectorEnum(&ENUM_, #ENUM_, ENUMNAME_, DEFINITION_FILE_);