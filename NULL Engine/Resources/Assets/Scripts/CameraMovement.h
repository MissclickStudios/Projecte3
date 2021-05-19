#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

class Player;

class SCRIPTS_API CameraMovement : public Script {
public:
	CameraMovement();
	~CameraMovement();

	void Start() override;
	void Update() override;
	void CleanUp()override;

	void CameraShake(float duration, float magnitude);
	void MoveCameraTo(GameObject* destination, float progress);

	//Shake variables
	float shakeDuration = 0.0f;
	float shakeMagnitude = 0.0f;
	//Player variables
	Player* playerScript;
	GameObject* player = nullptr;
	std::string playerName = "Mandalorian";

	//Camera
	float3 offset = float3::zero;
	Quat initialRot = Quat::identity;
	GameObject* destinationPoints;
	float nextPoint = 0.0f;
	float nextPointProgress = 0.0f;
	float cameraSpeed = 0.0f;
	float distanceToTransition = 0.0f;
	std::string destinationPointsName = "DestinationPoints";
};

SCRIPTS_FUNCTION CameraMovement* CreateCameraMovement() {
	CameraMovement* script = new CameraMovement();

	INSPECTOR_DRAGABLE_FLOAT3(script->offset);
	INSPECTOR_STRING(script->playerName);
	INSPECTOR_INPUT_FLOAT(script->shakeMagnitude);
	INSPECTOR_INPUT_FLOAT(script->shakeDuration);
	INSPECTOR_INPUT_FLOAT(script->cameraSpeed);
	INSPECTOR_INPUT_FLOAT(script->distanceToTransition);
	INSPECTOR_STRING(script->destinationPointsName);
	return script;
}