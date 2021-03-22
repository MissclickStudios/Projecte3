#include "JSONParser.h"
#include "Log.h"
#include "Timer.h"

#include "Application.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"
#include "M_Camera3D.h"
#include "M_Window.h"
#include "M_Editor.h"
#include "M_Audio.h"

#include "GameObject.h"

#include "C_Mesh.h"
#include "C_Animator.h"
#include "C_RigidBody.h"
#include "C_Transform.h"
#include "C_PlayerController.h"
#include "C_BoxCollider.h"
#include "C_BulletBehavior.h"
#include "C_Camera.h"
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
	stepTimer.Stop();
}

C_PlayerController::~C_PlayerController()
{
}

bool C_PlayerController::Update()
{
	if (App->gameState != GameState::PLAY)
		return true;

	if (!playAnim)
	{
		aAnimator = GetOwner()->GetComponent<C_Animator>();

		aAnimator->PlayClip("Idle", 0);
		playAnim = true;
	}

	AnimatorClip* currentClip = aAnimator->GetCurrentClip();
	std::string clipName = (currentClip != nullptr) ? currentClip->GetName() : "[NONE]";

	switch (state)
	{
	case PlayerState::IDLE:
		if (currentClip != nullptr && clipName != "Idle")
		{
			aAnimator->PlayClip("Idle", 0);
		}
		break;
	case PlayerState::RUNNING:
		if (currentClip != nullptr && clipName != "Running")
		{
			aAnimator->PlayClip("Running", 0);
		}
		break;
	case PlayerState::DASHING:
		if (currentClip != nullptr && clipName != "Dashing")
		{
			//aAnimator->PlayClip("Dashing", 0);
		}
		break;
	case PlayerState::SHOOTING:
		if (currentClip != nullptr && clipName != "Shooting")
		{
			aAnimator->PlayClip("Shooting", 0);
		}
		break;
	}

	if (!bulletStorage)
	{
		bulletStorage = App->scene->CreateGameObject("Bullets", App->scene->GetSceneRoot());
		for (uint i = 0; i < BULLET_AMOUNT; ++i)
			bullets[i] = CreateBullet(i);
	}

	Movement();
	Weapon();
	HandleHp();

	HandleAmmo(ammo);


	return true;
}

bool C_PlayerController::CleanUp()
{
	if (bulletStorage)
	{
		for (uint i = 0; i < BULLET_AMOUNT; ++i)
		{
			delete bullets[i];
			bullets[i] = nullptr;
		}
	
		App->scene->DeleteGameObject(bulletStorage);
		bulletStorage = nullptr;
	}

	return true;
}

void SetNullptr(GameObject* object)
{
	object = nullptr;
}

bool C_PlayerController::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("Speed", (double)speed);
	root.SetNumber("Acceleration", (double)acceleration);
	root.SetNumber("Deceleration", (double)deceleration);

	root.SetNumber("Bullet Speed", (double)bulletSpeed);
	root.SetNumber("Fire Rate", (double)fireRate);
	root.SetNumber("Ammo", (double)ammo);
	root.SetNumber("Max Ammo", (double)maxAmmo);
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
	ammo = (float)root.GetNumber("Ammo");
	maxAmmo = (float)root.GetNumber("Max Ammo");
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
				movY = -MAX_JOYSTICK_INPUT;
			if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)
				movY = MAX_JOYSTICK_INPUT;
			if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)
				movX = MAX_JOYSTICK_INPUT;
			if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)
				movX = -MAX_JOYSTICK_INPUT;
		}
		Move(rigidBody, movX, movY);

		if (dashColdown.IsActive())
		{
			if (dashColdown.ReadSec() >= dashingColdown)
				dashColdown.Stop();
		}
		else if ((App->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_DOWN || App->input->GetGameControllerTrigger(0) == ButtonState::BUTTON_DOWN))
			Dash(rigidBody, movX, movY);
	}
	else if (dashTime.ReadSec() >= dashingTime)
		dashTime.Stop();
}

void C_PlayerController::Move(C_RigidBody* rigidBody, int axisX, int axisY)
{
	float3 direction = { (float)axisX, 0, (float)axisY };

	if (axisX == 0 && axisY == 0) { state = PlayerState::IDLE; }
	else
	{
		state = PlayerState::RUNNING;
		
		direction.Normalize();
		lastDirection = direction;
		StepSound();
	}

	direction *= speed;
	rigidBody->SetLinearVelocity(direction);
}

void C_PlayerController::Dash(C_RigidBody* rigidBody, int axisX, int axisY)
{
	state = PlayerState::DASHING;
	
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
	{
		lastAim = lastDirection;
	}
	else
	{
		direction.Normalize();
		lastAim = direction;
	}

	if (GetOwner()->childs.size())
	{
		GameObject* mesh = GetOwner()->childs[0];
		if (mesh)
		{
			float2 dir = { lastAim.x, -lastAim.z };
			float rad = dir.AimedAngle();
			mesh->transform->SetLocalRotation(float3( 0, rad ,0 ));
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN)
		Reload();
	if (ammo > 0)
	{	
		if (!automatic)
		{
			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_DOWN || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_DOWN)
			{
				FireBullet(lastAim);
			}

			state = PlayerState::SHOOTING;
		}
		else
		{
			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_REPEAT || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_REPEAT)
			{
				if (!fireRateTimer.IsActive())
				{
					FireBullet(lastAim);
					fireRateTimer.Start();
				}
				else if (fireRateTimer.ReadSec() >= fireRate)
				{
					FireBullet(lastAim);
					fireRateTimer.Stop();
					fireRateTimer.Start();
				}

				state = PlayerState::SHOOTING;
			}
		}
	}
}

Bullet* C_PlayerController::CreateBullet(uint index)
{
	Resource* resource = App->resourceManager->GetResourceFromLibrary("Assets/Models/Props/bullet.fbx");

	GameObject* bullet;
	if (!resource)
		bullet = App->scene->CreateGameObject("Bullets", bulletStorage);
	else
		bullet = App->scene->GenerateGameObjectsFromModel((R_Model*)resource, { 0.5, 0.5, 0.5 });

	char n[10];
	sprintf_s(n, "%d", index);
	std::string num = n;
	std::string name("Bullet" + num);

	bullet->SetName(name.c_str());
	bullet->SetParent(bulletStorage);

	float3 position = float3::zero;
	bullet->transform->SetWorldPosition(position);

	C_RigidBody* rigidBody = (C_RigidBody*)bullet->CreateComponent(ComponentType::RIGIDBODY);
	rigidBody->FreezePositionY(true);
	rigidBody->FreezeRotationX(true);
	rigidBody->FreezeRotationY(true);
	rigidBody->FreezeRotationZ(true);
	((C_BoxCollider*)bullet->CreateComponent(ComponentType::BOX_COLLIDER))->SetTrigger(true);
	((C_BulletBehavior*)bullet->CreateComponent(ComponentType::BULLET_BEHAVIOR))->SetShooter(GetOwner(), index);

	bullet->CreateComponent(ComponentType::AUDIOSOURCE);
	C_AudioSource* source = bullet->GetComponent<C_AudioSource>();
	source->SetEvent("Mando_blaster_shot", App->audio->eventMap.at("Mando_blaster_shot"));

	for (uint i = 0; i < bullet->components.size(); ++i)
		bullet->components[i]->SetIsActive(false);
	bullet->SetIsActive(false);

	return new Bullet(bullet);
}

void C_PlayerController::FireBullet(float3 direction)
{
	if (direction.IsZero())
		++direction.z;

	GameObject* bullet = nullptr;

	for(uint i = 0; i < BULLET_AMOUNT; ++i)
		if (!bullets[i]->inUse)
		{
			bullets[i]->inUse = true;
			bullet = bullets[i]->object;
			break;
		}
	if (!bullet)
		return;

	float3 position = GetOwner()->transform->GetWorldPosition();
	position.y += 4;
	bullet->transform->SetWorldPosition(position);


	float2 dir = { lastAim.x, -lastAim.z };
	float rad = dir.AimedAngle();
	bullet->transform->SetLocalRotation(float3(0, rad, 0));


	bullet->GetComponent<C_BulletBehavior>()->StartAutodestructTimer();

	C_RigidBody* rigidBody = bullet->GetComponent<C_RigidBody>();
	rigidBody->TransformMovesRigidBody(true);
	rigidBody->SetLinearVelocity(direction * bulletSpeed);

	C_AudioSource* source = bullet->GetComponent<C_AudioSource>();
	source->PlayFx(source->GetEventId());

	bullet->SetIsActive(true);
	for (uint i = 0; i < bullet->components.size(); ++i)
		bullet->components[i]->SetIsActive(true);

	--ammo;
}

void C_PlayerController::Reload()
{
	ammo = maxAmmo;
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

void C_PlayerController::StepSound()
{
	if (!isStepPlaying)
	{
		isStepPlaying = true;
		stepTimer.Start();
	
		aSource = GetOwner()->GetComponent<C_AudioSource>();
		if (aSource != nullptr)
		{
			uint id = aSource->GetEventId();
			aSource->PlayFx(id);
		}
	}
	else
		if (stepTimer.ReadSec() >= 0.80)
			isStepPlaying = false;
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

void C_PlayerController::HandleAmmo(int ammo)
{
	std::vector<GameObject*>::iterator it = App->scene->GetGameObjects()->begin();

	for (it; it != App->scene->GetGameObjects()->end(); ++it)
	{
		if (strstr((*it)->GetName(), "PrimaryWeapon") != nullptr)
		{			
			ammoUi = (*it);
		}
	}

	R_Texture* ammo10 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo10.png");
	R_Texture* ammo9 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo9.png");
	R_Texture* ammo8 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo8.png");
	R_Texture* ammo7 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo7.png");
	R_Texture* ammo6 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo6.png");
	R_Texture* ammo5 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo5.png");
	R_Texture* ammo4 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo4.png");
	R_Texture* ammo3 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo3.png");
	R_Texture* ammo2 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo2.png");
	R_Texture* ammo1 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo1.png");
	R_Texture* ammo0 = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/Numbers/Ammo0.png");

	if (ammoUi != nullptr)
	{
		switch (ammo)
		{
		default: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo10); } break;
		case 9: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo9); } break;
		case 8: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo8); } break;
		case 7: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo7); } break;
		case 6: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo6); } break;
		case 5: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo5); } break;
		case 4: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo4); } break;
		case 3: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo3); } break;
		case 2: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo2); } break;
		case 1: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo1); } break;
		case 0: {ammoUi->GetComponent<C_Material>()->SwapTexture(ammo0); } break;
		}
	}
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
