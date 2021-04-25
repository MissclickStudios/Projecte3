#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float3.h"

class SCRIPTS_API CameraMovement : public Script {
public:
	CameraMovement();
	~CameraMovement();

	void Start() override;
	void Update() override;
	void CleanUp()override;

	GameObject* player = nullptr;
	std::string playerName = "Mandalorian";
	float3 offset = float3::zero;
};

SCRIPTS_FUNCTION CameraMovement* CreateCameraMovement() {
	CameraMovement* script = new CameraMovement();

	INSPECTOR_DRAGABLE_FLOAT3(script->offset);
	INSPECTOR_STRING(script->playerName);

	return script;
}