#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

class Player;
class IG12;
class GameManager;

class SCRIPTS_API CameraMovement : public Script {
public:
	CameraMovement();
	~CameraMovement();

	void Start() override;
	void Update() override;
	void CleanUp()override;

	void CameraShake(float duration, float magnitude);
	bool MoveCameraTo(GameObject* destination, float& progress);

	//Shake variables
	float shakeDuration = 0.0f;
	float shakeMagnitude = 0.0f;
	//Player variables
	Player* playerScript;
	IG12* ig12Script;
	GameObject* player = nullptr;
	std::string playerName = "Mandalorian";

	//Camera
	float3 offset = float3::zero;
	Quat initialRot = Quat::identity;
	float nextPoint = 0.0f;
	float nextPointProgress = 0.0f;
	float cameraSpeed = 0.0f;
	float distanceToTransition = 0.0f;
	GameObject* destinationPoints;
	std::string destinationPointsName = "DestinationPoints";

	GameObject* playerDestinationPoints;
	std::string playerPointsName = "PlayerPoints";
	//Game Manager

	std::string gameManagerName = "Game Manager";
	GameObject* gameManagerObject = nullptr;
	GameManager* gameManagerScript = nullptr;

};

SCRIPTS_FUNCTION CameraMovement* CreateCameraMovement();