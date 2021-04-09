#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float3.h"

class SCRIPTS_API CameraMovement : public Script {
public:
	CameraMovement();
	~CameraMovement();

	void Update() override;
	void CleanUp()override;

	GameObject* player = nullptr;

	float3 offset = float3::zero;
};

SCRIPTS_FUNCTION CameraMovement* CreateCameraMovement() {
	CameraMovement* script = new CameraMovement();

	//INSPECTOR_GAMEOBJECT(script->player);
	INSPECTOR_DRAGABLE_FLOAT(script->offset.x);
	INSPECTOR_DRAGABLE_FLOAT(script->offset.y);
	INSPECTOR_DRAGABLE_FLOAT(script->offset.z);

	return script;
}