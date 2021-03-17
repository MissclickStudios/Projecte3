#include "JSONParser.h"
#include "Log.h"
#include "Timer.h"

#include "Application.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_RigidBody.h"
#include "C_Mesh.h"

#include "C_RigidBody.h"
#include "C_Transform.h"
#include "C_PlayerController.h"
#include "C_Camera.h"
#include "M_Camera3D.h"
#include "M_Window.h"
#include "M_Editor.h"
#include "M_Audio.h"
#include "C_AudioSource.h"

#include "MathGeoLib/include/Geometry/Line.h"

#define MAX_JOYSTICK_INPUT 32767


C_PlayerController::C_PlayerController(GameObject* owner) : Component(owner, ComponentType::PLAYER_CONTROLLER)
{
	if (!GetOwner()->GetComponent<C_RigidBody>())
		GetOwner()->CreateComponent(ComponentType::RIGIDBODY);

	dashTime.Stop();
	dashColdown.Stop();
	//stepTimer = new Timer();
}

C_PlayerController::~C_PlayerController()
{
}

bool C_PlayerController::Update()
{
	if (!App->play || App->pause)
		return true;

	Movement();

	playerDirection = ReturnPlayerDirection();

	switch(playerDirection)
	{
	case Direction::NORTH:
		GetOwner()->transform->SetLocalEulerRotation(float3(0, 0, 0));
	break;
	case Direction::NORTH_WEST:
		GetOwner()->transform->SetLocalEulerRotation(float3(0, 45, 0));
		break;
	case Direction::WEST:
		GetOwner()->transform->SetLocalEulerRotation(float3(0, 90, 0));
		break;
	case Direction::SOUTH_WEST:
		GetOwner()->transform->SetLocalEulerRotation(float3(0, 135, 0));
		break;
	case Direction::SOUTH:
		GetOwner()->transform->SetLocalEulerRotation(float3(0, 180, 0));
		break;
	case Direction::SOUTH_EAST:
		GetOwner()->transform->SetLocalEulerRotation(float3(0, 225, 0));
		break;
	case Direction::EAST:
		GetOwner()->transform->SetLocalEulerRotation(float3(0, 270, 0));
		break;
	case Direction::NORTH_EAST:
		GetOwner()->transform->SetLocalEulerRotation(float3(0, 315, 0));
		break;
	}

	float3 ownerRotation = GetOwner()->transform->GetLocalEulerRotation();
	float3 bulletVel = { bulletSpeed * math::Cos(DegToRad(ownerRotation.x)) , 0, bulletSpeed * math::Sin(DegToRad(ownerRotation.x)) };

	if (App->input->GetMouseButton(1) == KeyState::KEY_DOWN || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_DOWN)
	{
		Resource* resource = App->resourceManager->GetResourceFromLibrary("Assets/Models/Primitives/sphere.fbx");
		if (resource != nullptr)
		{
			GameObject* bullet = App->scene->GenerateGameObjectsFromModel((R_Model*)resource);
	
			bullet->transform->SetWorldPosition(GetOwner()->transform->GetWorldPosition());
			C_RigidBody* rigidBody = (C_RigidBody*)bullet->CreateComponent(ComponentType::RIGIDBODY);
			rigidBody->FreezePositionY(true);
			rigidBody->FreezeRotationX(true);
			rigidBody->FreezeRotationY(true);
			rigidBody->FreezeRotationZ(true);
			rigidBody->SetLinearVelocity(bulletVel);
			bullet->CreateComponent(ComponentType::SPHERE_COLLIDER);
			bullet->CreateComponent(ComponentType::BULLET_BEHAVIOR);

		}
	}

	return true;
}

bool C_PlayerController::CleanUp()
{
	return true;
}

bool C_PlayerController::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("Speed", (double)speed);
	root.SetNumber("Acceleration", (double)acceleration);
	root.SetNumber("Deceleration", (double)deceleration);

	root.SetNumber("Bullet Speed", (double)bulletSpeed);

	root.SetNumber("Dash Speed", (double)dashSpeed);
	root.SetNumber("Dash Time", (double)dashingTime);
	root.SetNumber("Dash Coldown", (double)dashingColdown);

	return true;
}

bool C_PlayerController::LoadState(ParsonNode& root)
{
	speed = (float)root.GetNumber("Speed");
	acceleration = (float)root.GetNumber("Acceleration");
	deceleration = (float)root.GetNumber("Deceleration");

	bulletSpeed = (float)root.GetNumber("Bullet Speed");

	dashSpeed = (float)root.GetNumber("Dash Speed");
	dashingTime = (float)root.GetNumber("Dash Time");
	dashingColdown = (float)root.GetNumber("Dash Coldown");

	return true;
}

Direction C_PlayerController::ReturnPlayerDirection()
{
	bool north = false;
	bool west = false;
	bool south = false;
	bool east = false;

	if (App->input->GetGameControllerAxis(3) == AxisState::POSITIVE_AXIS_REPEAT)
		north = true;
	if (App->input->GetGameControllerAxis(3) == AxisState::NEGATIVE_AXIS_REPEAT)
		south = true;
	if (App->input->GetGameControllerAxis(2) == AxisState::POSITIVE_AXIS_REPEAT)
		west = true;
	if (App->input->GetGameControllerAxis(2) == AxisState::NEGATIVE_AXIS_REPEAT)
		east = true;

	if (north && west)
		return Direction::NORTH_WEST;
	if (north && east)
		return Direction::NORTH_EAST;
	if (south && west)
		return Direction::SOUTH_WEST;
	if (south && east)
		return Direction::SOUTH_EAST;

	if (north)
		return Direction::NORTH;
	if (south)
		return Direction::SOUTH;
	if (west)
		return Direction::WEST;
	if (east)
		return Direction::EAST;

	// we need to return last Direction
	return Direction::NORTH;
}

void C_PlayerController::Movement()
{
	C_RigidBody* rigidBody = GetOwner()->GetComponent<C_RigidBody>();
	if (!rigidBody || rigidBody->IsStatic())
		return;

	if (!dashTime.IsActive())
	{
		int movX = 0;
		int movY = 0;
		// Controller movement
		GetMovementVectorAxis(movX, movY);
		// Keyboard movement
		if (movX + movY == 0)
		{
			if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT)
				movY = MAX_JOYSTICK_INPUT;
			if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)
				movY = -MAX_JOYSTICK_INPUT;
			if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)
				movX = -MAX_JOYSTICK_INPUT;
			if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)
				movX = MAX_JOYSTICK_INPUT;
		}
		Move(rigidBody, movX, movY);

		if (dashColdown.IsActive())
		{
			if (dashColdown.ReadSec() >= dashingColdown)
				dashColdown.Stop();
		}
		else if ((App->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN))
			Dash(rigidBody, movX, movY);
	}
	else if (dashTime.ReadSec() >= dashingTime)
		dashTime.Stop();
}

void C_PlayerController::Move(C_RigidBody* rigidBody, int axisX, int axisY)
{
	float3 direction = { (float)axisX, 0, (float)axisY };

	if (axisX == 0 && axisY == 0) {}
	else
	{
		direction.Normalize();
		lastDirection = direction;
	}

	direction *= speed;
	rigidBody->SetLinearVelocity(direction);
}

void C_PlayerController::Dash(C_RigidBody* rigidBody, int axisX, int axisY)
{
	rigidBody->SetLinearVelocity(lastDirection * dashSpeed);

	dashColdown.Start();
	dashTime.Start();
}

void C_PlayerController::Rotate()
{
	//float2 mouse, center, direction;
	//mouse = MousePositionToWorldPosition();
	//center.x = GetOwner()->transform->GetWorldPosition().x;
	//center.y = GetOwner()->transform->GetWorldPosition().z;
	//mouse.y *= -1;
	//direction = mouse - center;
	//direction.Normalize();
	//
	//float rad = direction.AimedAngle();
	//
	//
	//float angle = RadToDeg(-rad) + 90;
	//GetOwner()->transform->SetLocalEulerRotation(float3(0, angle, 0));
}

float2 C_PlayerController::MousePositionToWorldPosition(float mapPositionY)
{
	float2 mousePos = float2(App->input->GetMouseX(), App->input->GetMouseY());

	float normMouseX = mousePos.x / (float)App->window->GetWidth();
	float normMouseY = mousePos.y / (float)App->window->GetHeight();

	float rayOriginX = (normMouseX - 0.5f) * 2;
	float rayOriginY = (normMouseY - 0.5f) * 2;

	LineSegment ray = App->camera->currentCamera->GetFrustum().UnProjectLineSegment(rayOriginX, rayOriginY);
	float3 direction = ray.Dir();
	float3 point = ray.AnyPointFast();

	float2 position = float2::zero;
	position.x = (-1 * direction.x * point.y) / direction.y + point.x;
	position.y = (-1 * direction.z * point.y) / direction.y + point.z;

	return position;
}

void C_PlayerController::StepSound(bool a, bool b, bool c, bool d)
{
	//if (a || b || c || d)
	//	if (!isStepPlaying)
	//	{
	//		isStepPlaying = true;
	//		stepTimer->Start();
	
	//		aSource = GetOwner()->GetComponent<C_AudioSource>();
	//		if (aSource != nullptr)
	//		{
	//			unsigned int id; aSource->GetEvent(nullptr, &id);
	//			aSource->PlayFx(id);
	//		}
	//	
	// 		if (isStepPlaying && stepTimer->ReadSec() >= 0.80)
	// 			isStepPlaying = false;
	//	}
}

void C_PlayerController::GetMovementVectorAxis(int& axisX, int& axisY)
{
	axisX = App->input->GetGameControllerAxisValue(0);
	axisY = App->input->GetGameControllerAxisValue(1);
}

void C_PlayerController::GetAimVectorAxis(int& axisX, int& axisY)
{
	axisX = App->input->GetGameControllerAxisValue(2);
	axisY = App->input->GetGameControllerAxisValue(3);
}