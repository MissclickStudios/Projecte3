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


C_PlayerController::C_PlayerController(GameObject* owner) : Component(owner, ComponentType::PLAYER_CONTROLLER)
{
	if (!GetOwner()->GetComponent<C_RigidBody>())
		GetOwner()->CreateComponent(ComponentType::RIGIDBODY);

	//stepTimer = new Timer();
}

C_PlayerController::~C_PlayerController()
{
}

bool C_PlayerController::Update()
{
	if (App->play && !App->pause)
	{
		C_RigidBody* rigidBody = GetOwner()->GetComponent<C_RigidBody>();

		if (rigidBody && !rigidBody->IsStatic())
		{
			int axisX = 0;
			int axisY = 0;
			// Keyboard movement
			if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT)
				axisY += 32000;
			if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)
				axisY += -32000;
			if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)
				axisX += -32000;
			if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)
				axisX += 32000;
			// Controller movement
			if (App->input->GetGameControllerAxis(1) == AxisState::POSITIVE_AXIS_REPEAT)
				axisY = 32000;
			if (App->input->GetGameControllerAxis(1) == AxisState::NEGATIVE_AXIS_REPEAT)
				axisY = -32000;
			if (App->input->GetGameControllerAxis(0) == AxisState::NEGATIVE_AXIS_REPEAT)
				axisX = -32000;
			if (App->input->GetGameControllerAxis(0) == AxisState::POSITIVE_AXIS_REPEAT)
				axisX = 32000;

			Move(rigidBody, axisX, axisY);

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
			//
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
		}
		else
			if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_DOWN || 
				App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_DOWN ||
				App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_DOWN ||
				App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_DOWN)
				LOG("Player controller error! No RigidBody found!");
	
		if (dashTimer == 0)
		{
			dashTimer = 0;
		}
		else 
		{
			dashTimer--;
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

	return true;
}

bool C_PlayerController::LoadState(ParsonNode& root)
{
	speed = (float)root.GetNumber("Speed");
	acceleration = (float)root.GetNumber("Acceleration");
	deceleration = (float)root.GetNumber("Deceleration");

	bulletSpeed = (float)root.GetNumber("Bullet Speed");

	return true;
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

float2 C_PlayerController::GetMovementVectorAxis()
{
	float2 movement = float2::zero;
	movement.x = App->input->GetGameControllerAxisValue(0) / 32767;
	movement.y = App->input->GetGameControllerAxisValue(1) / 32767;
	return movement;
}

float2 C_PlayerController::GetAimVectorAxis()
{
	float2 aim = float2::zero;
	aim.x = App->input->GetGameControllerAxisValue(2) / 32767;
	aim.y = App->input->GetGameControllerAxisValue(3) / 32767;
	return aim;
}

void C_PlayerController::Dash(C_RigidBody* rigidBody, bool forward, bool backward, bool right, bool left)
{
	if (!forward && !backward && right && !left) 
	{
		rigidBody->AddForce(physx::PxVec3(-dashForce, 0, 0), physx::PxForceMode::eIMPULSE);
		rightDash = true;
		dashTimer = dashCooldown;
	}

	if (!forward && !backward && !right && left)
	{
		rigidBody->AddForce(physx::PxVec3(dashForce, 0, 0), physx::PxForceMode::eIMPULSE);
		leftDash = true;
		dashTimer = dashCooldown;
	}
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

void C_PlayerController::Move(C_RigidBody* rigidBody, int axisX, int axisY)
{
	bool forward = false;
	bool backward = false;
	bool right = false;
	bool left = false;

	if ((App->input->GetKey(SDL_SCANCODE_E) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN ) && right && dashTimer == 0)
	{	
		Dash(rigidBody, forward, backward, right, left);
	}

	if ((App->input->GetKey(SDL_SCANCODE_E) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN) && left && dashTimer == 0)
	{
		Dash(rigidBody, forward, backward, right, left);
	}

	float3 direction = { (float)axisX, 0, (float)axisY };
	if (axisX + axisY != 0)
		direction.Normalize();
	direction *= speed;

	rigidBody->SetLinearVelocity(direction);
}

// void C_PlayerController::StepSound(bool a, bool b, bool c, bool d)
// {
// 	if (a || b || c || d)
// 	{
// 		if (!isStepPlaying)
// 		{
// 			isStepPlaying = true;
// 			stepTimer->Start();

// 			aSource = GetOwner()->GetComponent<C_AudioSource>();
// 			if (aSource != nullptr){
// 				unsigned int id; aSource->GetEvent(nullptr, &id);
// 				aSource->PlayFx(id);
// 			}
// 		}
// 	}

// 	if (isStepPlaying && stepTimer->ReadSec() >= 0.80)
// 	{
// 		isStepPlaying = false;
// 	}
// }

void C_PlayerController::Rotate()
{

}
