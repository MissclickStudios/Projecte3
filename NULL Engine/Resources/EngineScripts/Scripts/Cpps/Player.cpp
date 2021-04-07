#include "Player.h"
#include "Log.h"

#include "Application.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"
#include "M_Camera3D.h"
#include "M_Window.h"
#include "M_Editor.h"
#include "M_Audio.h"

#include "R_Texture.h"

#include "GameObject.h"

#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Animator.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"
#include "C_Camera.h"
#include "C_AudioSource.h"
#include "C_Material.h"

#include "C_Script.h"
#include "R_Script.h"
#include "Bullet.h"

#include "MathGeoLib/include/Geometry/Line.h"

#define MAX_JOYSTICK_INPUT 32767

enum class PlayerState
{
	IDLE,
	RUNNING,
	DASHING,
	SHOOTING
};

Player::Player() : Script(), state(PlayerState::IDLE)
{
	if (!gameObject->GetComponent<C_RigidBody>())
		gameObject->CreateComponent(ComponentType::RIGIDBODY);

	fireRateTimer.Stop();
	dashTime.Stop();
	dashColdown.Stop();
	stepTimer.Stop();

	//memset(ammoTex, 0, 11 * sizeof(R_Texture*));
	ammo = 10;
}

Player::~Player()
{
}

void Player::Update()
{
	if (App->gameState != GameState::PLAY)
		return;

	if (!playAnim)
	{
		aAnimator = gameObject->GetComponent<C_Animator>();

		//aAnimator->PlayClip("Idle", 0);
		playAnim = true;
	}

	AnimatorClip* currentClip = aAnimator->GetCurrentClip();
	std::string clipName = (currentClip != nullptr) ? currentClip->GetName() : "[NONE]";

	switch (state)
	{
	case PlayerState::IDLE:
		if (currentClip != nullptr && clipName != "Idle")
		{
			//aAnimator->PlayClip("Idle", 0);
		}
		break;
	case PlayerState::RUNNING:
		if (currentClip != nullptr && clipName != "Running4")
		{
			//aAnimator->PlayClip("Running4", 0);
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
			//aAnimator->PlayClip("Shooting", 0);
		}
		break;
	}

	if (!bulletStorage)
	{
		bulletStorage = App->scene->CreateGameObject("Bullets", App->scene->GetSceneRoot());
		for (uint i = 0; i < BULLET_AMOUNT; ++i)
			bullets[i] = CreateProjectile(i);
	}

	Movement();
	Weapon();
	HandleHp();

	HandleAmmo(ammo);
}

void Player::CleanUp()
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

	if (storedAmmoTex)
	{
		for (uint i = 0; i < 11; ++i)
		{
			if (ammoTex[i] != 0)
			{
				App->resourceManager->FreeResource(ammoTex[i]->GetUID());
			}
		}
	}

	if (full != nullptr) { App->resourceManager->FreeResource(full->GetUID()); }
	if (half != nullptr) { App->resourceManager->FreeResource(half->GetUID()); }
	if (empty != nullptr) { App->resourceManager->FreeResource(empty->GetUID()); }
}

void Player::Movement()
{
	C_RigidBody* rigidBody = gameObject->GetComponent<C_RigidBody>();
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

void Player::Move(C_RigidBody* rigidBody, int axisX, int axisY)
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

void Player::Dash(C_RigidBody* rigidBody, int axisX, int axisY)
{
	state = PlayerState::DASHING;

	rigidBody->SetLinearVelocity(lastDirection * dashSpeed);

	dashColdown.Start();
	dashTime.Start();
}

void Player::Rotate()
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

void Player::Weapon()
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

	if (gameObject->childs.size())
	{
		GameObject* mesh = gameObject->childs[0];
		if (mesh)
		{
			float2 dir = { lastAim.x, -lastAim.z };
			float rad = dir.AimedAngle();
			mesh->transform->SetLocalRotation(float3(0, rad, 0));
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

Projectile* Player::CreateProjectile(uint index)
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
	rigidBody->ChangeFilter("bullet");
	bullet->CreateComponent(ComponentType::BOX_COLLIDER);

	C_Script* script = (C_Script*)bullet->CreateComponent(ComponentType::SCRIPT);
	script->resource = (R_Script*)App->resourceManager->GetResourceFromLibrary("Assets/Scripts//Bullet.h");
	for (int i = 0; i < script->resource->dataStructures.size(); ++i) {
		if (script->resource->dataStructures[i].first == "Bullet")
		{
			script->LoadData("Bullet", script->resource->dataStructures[i].second);
			((Bullet*)script->scriptData)->SetShooter(this, index);
			break;
		}
	}

	bullet->CreateComponent(ComponentType::PARTICLE_SYSTEM);

	bullet->CreateComponent(ComponentType::AUDIOSOURCE);
	C_AudioSource* source = bullet->GetComponent<C_AudioSource>();
	source->SetEvent("Mando_blaster_shot", App->audio->eventMap.at("Mando_blaster_shot"));

	for (uint i = 0; i < bullet->components.size(); ++i)
		bullet->components[i]->SetIsActive(false);
	bullet->SetIsActive(false);

	return new Projectile(bullet);
}

void Player::FireBullet(float3 direction)
{
	if (direction.IsZero())
		++direction.z;

	GameObject* bullet = nullptr;

	for (uint i = 0; i < BULLET_AMOUNT; ++i)
		if (!bullets[i]->inUse)
		{
			bullets[i]->inUse = true;
			bullet = bullets[i]->object;
			break;
		}
	if (!bullet)
		return;

	float3 position = gameObject->transform->GetWorldPosition();
	position.y += 4;
	bullet->transform->SetWorldPosition(position);


	float2 dir = { lastAim.x, -lastAim.z };
	float rad = dir.AimedAngle();
	bullet->transform->SetLocalRotation(float3(0, rad, 0));

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

void Player::Reload()
{
	ammo = maxAmmo;
}

float2 Player::MousePositionToWorldPosition(float mapPositionY)
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

void Player::StepSound()
{
	if (!isStepPlaying)
	{
		isStepPlaying = true;
		stepTimer.Start();
	
		aSource = gameObject->GetComponent<C_AudioSource>();
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

void Player::GetMovementVectorAxis(int& axisX, int& axisY)
{
	axisX = App->input->GetGameControllerAxisValue(0);
	axisY = App->input->GetGameControllerAxisValue(1);
}

void Player::GetAimVectorAxis(int& axisX, int& axisY)
{
	axisX = App->input->GetGameControllerAxisValue(2);
	axisY = App->input->GetGameControllerAxisValue(3);
}

void Player::HandleAmmo(int ammo)
{
	std::vector<GameObject*>::iterator it = App->scene->GetGameObjects()->begin();

	for (it; it != App->scene->GetGameObjects()->end(); ++it)
	{
		if (strstr((*it)->GetName(), "PrimaryWeapon") != nullptr)
		{
			ammoUi = (*it);
		}
	}

	if (!storedAmmoTex)
	{
		std::string ammoTexPath = "Assets/Textures/HUD/Numbers/Ammo";

		for (uint i = 0; i < 11; ++i)
		{
			std::string path = ammoTexPath + std::to_string(i) + ".png";
			ammoTex[i] = (R_Texture*)App->resourceManager->GetResourceFromLibrary(path.c_str());
		}

		storedAmmoTex = true;
	}

	if (ammoUi != nullptr)
	{
		C_Material* cMaterial = ammoUi->GetComponent<C_Material>();
		if (cMaterial == nullptr)
		{
			return;
		}

		cMaterial->SwapTexture(ammoTex[ammo]);
	}
}

void Player::HandleHp()
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

	if (full == nullptr) { full = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartFull.png"); }
	if (half == nullptr) { half = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartHalf.png"); }
	if (empty == nullptr) { empty = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartEmpty.png"); }

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

Player* CreatePlayer() 
{
	Player* script = new Player();
	return script;
}