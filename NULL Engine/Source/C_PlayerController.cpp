#include "JSONParser.h"
#include "Log.h"

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

#include "MathGeoLib/include/Geometry/Line.h"


C_PlayerController::C_PlayerController(GameObject* owner) : Component(owner, ComponentType::PLAYER_CONTROLLER)
{
	if (!GetOwner()->GetComponent<C_RigidBody>())
		GetOwner()->CreateComponent(ComponentType::RIGIDBODY);
}

C_PlayerController::~C_PlayerController()
{
}

bool C_PlayerController::Update()
{
	if (App->play && !App->pause)
	{
		C_RigidBody* rigidBody = GetOwner()->GetComponent<C_RigidBody>();
		if (rigidBody)
		{
			if (useAcceleration)
				MoveAcceleration(rigidBody);
			else
				MoveVelocity(rigidBody);

			// if (!cameraMode)
			// {
			// 	float2 mouse, center, direction;
			// 	mouse = MousePositionToWorldPosition();
			// 	center.x = GetOwner()->transform->GetWorldPosition().x;
			// 	center.y = GetOwner()->transform->GetWorldPosition().z;
			// 	mouse.y *= -1;
			// 	direction = mouse - center;
			// 	direction.Normalize();
			// 
			// 	float rad = direction.AimedAngle();
			// 	float3 bulletVel = { bulletSpeed * math::Cos(rad) , 0, bulletSpeed * math::Sin(rad) };
			// 
			// 	float angle = RadToDeg(-rad) + 90;
			// 	GetOwner()->transform->SetLocalEulerRotation(float3(0, angle, 0));
			// 
			// 	if (App->input->GetMouseButton(1) == KeyState::KEY_DOWN || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_DOWN)
			// 	{
			// 		Resource* resource = App->resourceManager->GetResourceFromLibrary("Assets/Models/Primitives/sphere.fbx");
			// 		if (resource != nullptr)
			// 		{
			// 			GameObject* bullet = App->scene->GenerateGameObjectsFromModel((R_Model*)resource);
			// 
			// 			bullet->transform->SetWorldPosition(GetOwner()->transform->GetWorldPosition());
			// 			C_RigidBody* rigidBody = (C_RigidBody*)bullet->CreateComponent(ComponentType::RIGIDBODY);
			// 			rigidBody->FreezePositionY(true);
			// 			rigidBody->FreezeRotationX(true);
			// 			rigidBody->FreezeRotationY(true);
			// 			rigidBody->FreezeRotationZ(true);
			// 			rigidBody->SetLinearVelocity(bulletVel);
			// 			bullet->CreateComponent(ComponentType::SPHERE_COLLIDER);
			// 			bullet->CreateComponent(ComponentType::BULLET_BEHAVIOR);
			// 		}
			// 	}
			// }
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

	root.SetBool("Use Acceleration", useAcceleration);

	root.SetNumber("Bullet Speed", (double)bulletSpeed);

	root.SetBool("Camera Mode", cameraMode);

	return true;
}

bool C_PlayerController::LoadState(ParsonNode& root)
{
	speed = (float)root.GetNumber("Speed");
	acceleration = (float)root.GetNumber("Acceleration");
	deceleration = (float)root.GetNumber("Deceleration");

	useAcceleration = root.GetBool("Use Acceleration");

	bulletSpeed = (float)root.GetNumber("Bullet Speed");

	cameraMode = root.GetBool("Camera Mode");

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

void C_PlayerController::MoveVelocity(C_RigidBody* rigidBody)
{
	float3 vel = float3::zero;

	bool forward = false;
	bool backwards = false;
	bool right = false;
	bool left = false;

	if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT)
		forward = true;
	if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)
		backwards = true;
	if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)
		right = true;
	if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)
		left = true;

	if (App->input->GetGameControllerAxis(1) == AxisState::POSITIVE_AXIS_REPEAT)
		forward = true;
	if (App->input->GetGameControllerAxis(1) == AxisState::NEGATIVE_AXIS_REPEAT)
		backwards = true;
	if (App->input->GetGameControllerAxis(0) == AxisState::POSITIVE_AXIS_REPEAT)
		right = true;
	if (App->input->GetGameControllerAxis(0) == AxisState::NEGATIVE_AXIS_REPEAT)
		left = true;

	if(App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_DOWN && right && dashTimer == 0)
	{
		rightDash = true;
		dashTimer = dashCooldown;
		rigidBody->AddForce(physx::PxVec3(dashForce, 0, 0), physx::PxForceMode::eIMPULSE);
	}
		
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_DOWN && left && dashTimer == 0)
	{
		leftDash = true;
		dashTimer = dashCooldown;
		rigidBody->AddForce(physx::PxVec3(-dashForce, 0, 0), physx::PxForceMode::eIMPULSE);
	}

	if (forward)
		vel.z += speed;
	if (backwards)
		vel.z -= speed;
	if (right)
		vel.x += speed;
	if (left)
		vel.x -= speed;

	rigidBody->SetLinearVelocity(vel);
}

void C_PlayerController::MoveAcceleration(C_RigidBody* rigidBody)
{
	float3 vel = rigidBody->GetLinearVelocity();

	bool forward = false;
	bool backward = false;
	bool right = false;
	bool left = false;
	if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT)
		forward = true;
	if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)
		backward = true;
	if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)
		right = true;
	if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)
		left = true;

	if (forward)
		rigidBody->AddForce(physx::PxVec3(0, 0, acceleration), physx::PxForceMode::eACCELERATION);
	else if (!backward)
		if (vel.z > 0)
			rigidBody->AddForce(physx::PxVec3(0, 0, -deceleration), physx::PxForceMode::eACCELERATION);

	if (backward)
		rigidBody->AddForce(physx::PxVec3(0, 0, -acceleration), physx::PxForceMode::eACCELERATION);
	else if (!forward)
		if (vel.z < 0)
			rigidBody->AddForce(physx::PxVec3(0, 0, deceleration), physx::PxForceMode::eACCELERATION);

	if (left)
		rigidBody->AddForce(physx::PxVec3(acceleration, 0, 0), physx::PxForceMode::eACCELERATION);
	else if (!right)
		if (vel.x > 0)
			rigidBody->AddForce(physx::PxVec3(-deceleration, 0, 0), physx::PxForceMode::eACCELERATION);

	if (right)
		rigidBody->AddForce(physx::PxVec3(-acceleration, 0, 0), physx::PxForceMode::eACCELERATION);
	else if (!left)
		if (vel.x < 0)
			rigidBody->AddForce(physx::PxVec3(deceleration, 0, 0), physx::PxForceMode::eACCELERATION);

	//bool changed = false;
	//for (int i = 0; i < 3; i++)
	//	if (vel[i] > speed)
	//	{
	//		vel[i] = speed;
	//		changed = true;
	//	}
	//	else if (vel[i] < -speed)
	//	{
	//		vel[i] = -speed;
	//		changed = true;
	//	}
	//if (changed)
	//	rigidBody->SetLinearVelocity(vel);
}

void C_PlayerController::Rotate()
{
}