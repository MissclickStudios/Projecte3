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
#include "C_Material.h"

#include "MathGeoLib/include/Geometry/Line.h"

#define MAX_JOYSTICK_INPUT 32767


C_PlayerController::C_PlayerController(GameObject* owner) : Component(owner, ComponentType::PLAYER_CONTROLLER)
{
	if (!GetOwner()->GetComponent<C_RigidBody>())
		GetOwner()->CreateComponent(ComponentType::RIGIDBODY);

	fireRateTimer.Stop();
	dashTime.Stop();
	dashColdown.Stop();
	//stepTimer = new Timer();
}

C_PlayerController::~C_PlayerController()
{
}

bool C_PlayerController::Update()
{
	if (App->gameState != GameState::PLAY)
		return true;

	Movement();
	Weapon();
	HandleHp();

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
	root.SetNumber("Fire Rate", (double)fireRate);
	root.SetBool("Automatic", automatic);

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
	fireRate = (float)root.GetNumber("Fire Rate");
	automatic = root.GetBool("Automatic");

	dashSpeed = (float)root.GetNumber("Dash Speed");
	dashingTime = (float)root.GetNumber("Dash Time");
	dashingColdown = (float)root.GetNumber("Dash Coldown");

	return true;
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

void C_PlayerController::Weapon()
{
	int aimX = 0;
	int aimY = 0;
	// Controller aim
	GetAimVectorAxis(aimX, aimY);
	// Mouse aim
	// TODO

	float3 direction = { (float)aimX, 0, (float)aimY };
	if (aimX == 0 && aimY == 0)
		direction.z++; // No mouse aim picking
	else
		direction.Normalize();

	if (!automatic)
	{
		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_DOWN || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_DOWN)
			SpawnBullet(direction);
	}
	else
	{
		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_REPEAT || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_REPEAT)
		{
			if (!fireRateTimer.IsActive())
			{
				SpawnBullet(direction);
				fireRateTimer.Start();
			}
			else if (fireRateTimer.ReadSec() >= fireRate)
			{
				SpawnBullet(direction);
				fireRateTimer.Stop();
				fireRateTimer.Start();
			}
		}
	}
}

void C_PlayerController::SpawnBullet(float3 direction)
{
	Resource* resource = App->resourceManager->GetResourceFromLibrary("Assets/Models/Primitives/sphere.fbx");
	if (!resource)
		return;

	GameObject* bullet = App->scene->GenerateGameObjectsFromModel((R_Model*)resource);

	bullet->transform->SetWorldPosition(GetOwner()->transform->GetWorldPosition());
	C_RigidBody* rigidBody = (C_RigidBody*)bullet->CreateComponent(ComponentType::RIGIDBODY);
	rigidBody->FreezePositionY(true);
	rigidBody->FreezeRotationX(true);
	rigidBody->FreezeRotationY(true);
	rigidBody->FreezeRotationZ(true);
	rigidBody->SetLinearVelocity(direction * bulletSpeed);
	bullet->CreateComponent(ComponentType::SPHERE_COLLIDER);
	bullet->CreateComponent(ComponentType::BULLET_BEHAVIOR);
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

void C_PlayerController::HandleHp()
{

	std::vector<GameObject*>::iterator it = App->scene->GetGameObjects()->begin();

	for (it; it != App->scene->GetGameObjects()->end(); ++it)
	{
		if (strstr((*it)->GetName(), "Heart") != nullptr)
		{
			if (strstr((*it)->GetName(), "1") != nullptr)
			{
				hearts[0] = (*it);
			}		
			else if (strstr((*it)->GetName(), "2") != nullptr)
			{
				hearts[1] = (*it);
			}
			else if (strstr((*it)->GetName(), "3") != nullptr)
			{
				hearts[2] = (*it);
			}
		}
	}

	R_Texture* half = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartHalf.png");
	R_Texture* full = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartFull.png");
	R_Texture* empty = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartEmpty.png");

	if (App->input->GetKey(SDL_SCANCODE_K) == KeyState::KEY_DOWN && hearts != nullptr)
	{
		heart -= 0.5;

		if (heart < 0)
			heart = 0;

		if (heart == 2.5)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(half);

		else if (heart == 2)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(empty);

		else if (heart == 1.5)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(half);

		else if (heart == 1)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(empty);

		else if (heart == 0.5)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(half);

		else if (heart == 0)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(empty);
	}

	if (App->input->GetKey(SDL_SCANCODE_L) == KeyState::KEY_DOWN && hearts != nullptr)
	{

		if (heart == 2.5)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(full);

		else if (heart == 2)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(half);

		else if (heart == 1.5)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(full);

		else if (heart == 1)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(half);

		else if (heart == 0.5)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(full);

		else if (heart == 0)
			hearts[(int)heart]->GetComponent<C_Material>()->SwapTexture(half);

		heart += 0.5;

		if (heart > 3)
			heart = 3;
	}
}
