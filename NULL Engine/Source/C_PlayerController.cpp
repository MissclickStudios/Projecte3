#include "JSONParser.h"

#include "Application.h"
#include "M_Input.h"
#include "Log.h"

#include "GameObject.h"
#include "C_RigidBody.h"

#include "C_RigidBody.h"
#include "C_Transform.h"
#include "C_PlayerController.h"
#include "C_Camera.h"
#include "M_Camera3D.h"
#include "M_Window.h"
#include "M_Editor.h"


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
		}
		else
			if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_DOWN || 
				App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_DOWN ||
				App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_DOWN ||
				App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_DOWN)
				LOG("Player controller error! No RigidBody found!");
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

	return true;
}

bool C_PlayerController::LoadState(ParsonNode& root)
{
	speed = (float)root.GetNumber("Speed");
	acceleration = (float)root.GetNumber("Acceleration");
	deceleration = (float)root.GetNumber("Deceleration");

	useAcceleration = root.GetBool("Use Acceleration");

	return true;
}

float3 C_PlayerController::MousePositionToWorldPosition(float mapPositionY)
{
	//float2 mousePos = App->editor->GetWorldMousePositionThroughEditor();
	float2 mousePos = float2(App->input->GetMouseX(), App->input->GetMouseY());

	float normMouseX = mousePos.x / (float)App->window->GetWidth();
	float normMouseY = mousePos.y / (float)App->window->GetHeight();

	float rayOriginX = (normMouseX - 0.5f) * 2;
	float rayOriginY = (normMouseY - 0.5f) * 2;

	LineSegment raycast = App->camera->currentCamera->GetFrustum().UnProjectLineSegment(rayOriginX, rayOriginY);

	return float3();
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
		rigidBody->AddForce(physx::PxVec3(0, 0, acceleration), physx::PxForceMode::eVELOCITY_CHANGE);
	else if (!backward)
		if (vel.z > 0)
			rigidBody->AddForce(physx::PxVec3(0, 0, -deceleration), physx::PxForceMode::eVELOCITY_CHANGE);

	if (backward)
		rigidBody->AddForce(physx::PxVec3(0, 0, -acceleration), physx::PxForceMode::eVELOCITY_CHANGE);
	else if (!forward)
		if (vel.z < 0)
			rigidBody->AddForce(physx::PxVec3(0, 0, deceleration), physx::PxForceMode::eVELOCITY_CHANGE);

	if (left)
		rigidBody->AddForce(physx::PxVec3(acceleration, 0, 0), physx::PxForceMode::eVELOCITY_CHANGE);
	else if (!right)
		if (vel.x > 0)
			rigidBody->AddForce(physx::PxVec3(-deceleration, 0, 0), physx::PxForceMode::eVELOCITY_CHANGE);

	if (right)
		rigidBody->AddForce(physx::PxVec3(-acceleration, 0, 0), physx::PxForceMode::eVELOCITY_CHANGE);
	else if (!left)
		if (vel.x < 0)
			rigidBody->AddForce(physx::PxVec3(deceleration, 0, 0), physx::PxForceMode::eVELOCITY_CHANGE);

	bool changed = false;
	for (int i = 0; i < 3; i++)
		if (vel[i] > speed)
		{
			vel[i] = speed;
			changed = true;
		}
		else if (vel[i] < -speed)
		{
			vel[i] = -speed;
			changed = true;
		}
	if (changed)
		rigidBody->SetLinearVelocity(vel);
}

void C_PlayerController::Rotate()
{
}