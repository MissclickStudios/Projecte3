#include "JSONParser.h"

#include "Application.h"
#include "M_Input.h"

#include "GameObject.h"
#include "C_RigidBody.h"

#include "C_RigidBody.h"
#include "C_Transform.h"
#include "C_PlayerController.h"

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
			float3 vel = float3::zero;

			bool forward = false;
			bool backwards = false;
			bool right = false;
			bool left = false;
			if (App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_REPEAT)
				forward = true;
			if (App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_REPEAT)
				backwards = true;
			if (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_REPEAT)
				right = true;
			if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_REPEAT)
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
		else
			if (App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_DOWN || 
				App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_DOWN ||
				App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_DOWN ||
				App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_DOWN)
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

	return true;
}

bool C_PlayerController::LoadState(ParsonNode& root)
{
	speed = (float)root.GetNumber("Speed");

	return true;
}

void C_PlayerController::Move()
{
}

void C_PlayerController::Rotate()
{
}