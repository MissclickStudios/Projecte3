#include "Player.h"

#include "VariableTypedefs.h"
#include "Log.h"

#include "Application.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"
#include "M_Camera3D.h"
#include "M_Window.h"
#include "M_Editor.h"
#include "M_Audio.h"
#include "M_Physics.h"

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

#include "Weapon.h"

#include "Time.h"

#include "MathGeoLib/include/Geometry/Line.h"

#define MAX_JOYSTICK_INPUT 32767

enum class PlayerState
{
	IDLE,
	RUNNING,
	DASHING,
	SHOOTING_BLASTER,
	SHOOTING_SNIPER,
	RELOADING_BLASTER,
	RELOADING_SNIPER,
	DEAD
};

Player::Player() : Script(), state(PlayerState::IDLE)
{
	blasterReloadTimer.Stop();
	sniperReloadTimer.Stop();
	shootAnimTimer.Stop();
}

Player::~Player()
{
}

void Player::Awake()
{
	if (!gameObject->GetComponent<C_RigidBody>())
		gameObject->CreateComponent(ComponentType::RIGIDBODY);

	std::map<uint32, GameObject*>* objects = App->scene->GetGameObjects();
	for (auto o = objects->begin(); o != objects->end(); ++o)
	{
		std::string name = o->second->GetName();
		LOG("%s", name.c_str());
		if (name == "Blaster")
			blasterModel = o->second;
		else if (name == "Sniper")
			sniperModel = o->second;
	}
	if (blasterModel)
		blasterModel->SetIsActive(true);
	if (sniperModel)
		sniperModel->SetIsActive(false);

	dashTime.Stop();
	dashColdown.Stop();
	stepTimer.Stop();
	invulnerabilityTimer.Stop();
}

void Player::Update()
{
	Animations();
	if (health <= 0.0f)
	{
		state = PlayerState::DEAD;
		C_RigidBody* rigidBody = gameObject->GetComponent<C_RigidBody>();
		if (!rigidBody || rigidBody->IsStatic())
			return;
		rigidBody->SetLinearVelocity(float3::zero);

		return;
	}

	Movement();

	if (!blaster)
		blaster = new Weapon(gameObject, blasterBullet, 10u, blasterMaxAmmo, blasterSpeed, blasterRate, blasterAutomatic);
	blaster->Update();
	if (!sniper)
		sniper = new Weapon(gameObject, sniperBullet, 2u, sniperMaxAmmo, sniperSpeed, sniperRate, sniperAutomatic);
	sniper->Update();

	if (strongShots)
		BlasterStrongShots();
	if (freezingShots)
		SniperFreezingShots();

	blasterAmmo = blaster->ammo;
	sniperAmmo = sniper->ammo;
	Shooting();

	if (App->input->GetKey(SDL_SCANCODE_K) == KeyState::KEY_DOWN && hearts != nullptr)
		health -= 0.5f;
	if (App->input->GetKey(SDL_SCANCODE_L) == KeyState::KEY_DOWN && hearts != nullptr)
		health += 0.5f;

	//HandleHp();
	//HandleAmmo(ammo);
}

void Player::CleanUp()
{
	delete blaster; // Destructor calls CleanUp
	blaster = nullptr;

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

void Player::TakeDamage(float damage)
{
	if (!invulnerabilityTimer.IsActive())
	{
		health -= damage;
		if (health < 0.0f)
			health = 0.0f;
		invulnerabilityTimer.Start();
	}
	else
	{
		if (invulnerabilityTimer.ReadSec() >= invulnerability)
			invulnerabilityTimer.Stop();
	}
}

void Player::BlasterStrongShots()
{
	strongShots = false;
	for (uint i = 0; i < blaster->projectilesNum; ++i)
		((Bullet*)blaster->projectiles[i]->object->GetScript("Bullet"))->strong = true;
}

void Player::SniperFreezingShots()
{
	freezingShots = false;
	for (uint i = 0; i < sniper->projectilesNum; ++i)
		((Bullet*)sniper->projectiles[i]->object->GetScript("Bullet"))->freeze = true;
}

void Player::Animations()
{
	if (!playAnim)
	{
		aAnimator = gameObject->GetComponent<C_Animator>();

		aAnimator->PlayClip("Idle", 0u);
		playAnim = true;
	}

	AnimatorClip* currentClip = aAnimator->GetCurrentClip();
	std::string clipName = (currentClip != nullptr) ? currentClip->GetName() : "[NONE]";

	if (shootAnimTimer.IsActive())
	{
		if (shootAnimTimer.ReadSec() >= currentClip->GetDurationInSeconds())
		{
			shootAnimTimer.Stop();
		}
		else
			return;
	}

	switch (state)
	{
	case PlayerState::IDLE:
		if (currentClip && clipName != idle)
			aAnimator->PlayClip(idle, 0.2f);
		break;
	case PlayerState::RUNNING:
		if (currentClip && clipName != walk)
			aAnimator->PlayClip(walk, 0.2f);
		break;
	//case PlayerState::DASHING:
	//	if (currentClip != nullptr && clipName != dash)
	//	{
	//		aAnimator->PlayClip(dash, 0u);
	//	}
	//	break;
	case PlayerState::SHOOTING_BLASTER:
		if (currentClip && clipName != shootBlaster)
		{
			aAnimator->PlayClip(shootBlaster, 0.1f);
			shootAnimTimer.Start();
		}
		break;
	case PlayerState::SHOOTING_SNIPER:
		if (currentClip && clipName != shootBlaster)
		{
			aAnimator->PlayClip(shootBlaster, 0.1f);
			shootAnimTimer.Start();
		}
		//if (currentClip != nullptr && clipName != shootSniper)
		//{
		//	aAnimator->PlayClip(shootSniper, 0u);
		//  shootAnimTimer.Start();
		//}
		break;
	case PlayerState::RELOADING_BLASTER:
	//	if (currentClip != nullptr && clipName != "Shooting")
	//	{
	//		//aAnimator->PlayClip("Reloading", 0u);
	//	}
		if (currentClip && clipName != idle)
			aAnimator->PlayClip(idle, 0.2f);
		break;
	case PlayerState::RELOADING_SNIPER:
	//	if (currentClip != nullptr && clipName != "Shooting")
	//	{
	//		//aAnimator->PlayClip("Reloading", 0u);
	//	}
		if (currentClip && clipName != idle)
			aAnimator->PlayClip(idle, 0.2f);
		break;
	//case PlayerState::DEAD:
	//	if (currentClip != nullptr && clipName != die)
	//	{
	//		aAnimator->PlayClip(die, 0u);
	//	}
	//	break;
	}
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
		{
			// 0 -> player filter
			// 4 -> enemy bullet filter
			App->physics->GetInteractions()[0][4] = false;
			App->physics->GetInteractions()[4][0] = false;

			state = PlayerState::DASHING;
			dashColdown.Start();
			dashTime.Start();
		}
	}
	else
	{
		rigidBody->SetLinearVelocity(lastDirection * dashSpeed);

		if (dashTime.ReadSec() >= dashingTime)
		{
			// 0 -> player filter
			// 4 -> enemy bullet filter
			App->physics->GetInteractions()[0][4] = true;
			App->physics->GetInteractions()[4][0] = true;

			dashTime.Stop();
		}
	}
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

void Player::Shooting()
{
	int aimX = 0;
	int aimY = 0;
	// Controller aim
	GetAimVectorAxis(aimX, aimY);
	// Keyboard movement
	if (aimX + aimY == 0)
	{
		if (App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_REPEAT)
			aimY = -MAX_JOYSTICK_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_REPEAT)
			aimY = MAX_JOYSTICK_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_REPEAT)
			aimX = MAX_JOYSTICK_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_REPEAT)
			aimX = -MAX_JOYSTICK_INPUT;
	}

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

	for (uint i = 0; i < gameObject->childs.size(); ++i)
	{
		float2 dir = { lastAim.x, -lastAim.z };
		float rad = dir.AimedAngle();
		if (!gameObject->childs[i]->GetComponent<C_Mesh>()) // FUCK MESHES ALL MY HOMIES HATE MESHES
			gameObject->childs[i]->transform->SetLocalRotation(float3(0, rad + DegToRad(90), 0));
	}

	if (App->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN)
		if (weaponUsed == 1)
		{
			weaponUsed = 2;
			if (blasterModel)
				blasterModel->SetIsActive(false);
			if (sniperModel)
				sniperModel->SetIsActive(true);
		}
		else
		{
			weaponUsed = 1;
			if (blasterModel)
				blasterModel->SetIsActive(true);
			if (sniperModel)
				sniperModel->SetIsActive(false);
		}

	bool shooting = false;
	if (weaponUsed == 1)
	{
		if (!blasterReloadTimer.IsActive())
		{
			if ((App->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN)
				|| blasterAmmo <= 0)
				blasterReloadTimer.Start();
			else
				if (blaster->Shoot(lastAim))
					state = PlayerState::SHOOTING_BLASTER;
		}
		else
		{
			state = PlayerState::RELOADING_BLASTER;
			if (blasterReloadTimer.ReadSec() >= blasterReloadTime)
			{
				blaster->Reload();
				blasterReloadTimer.Stop();
			}
		}
	}
	else
	{
		if (!sniperReloadTimer.IsActive())
		{
			if ((App->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN)
				|| sniperAmmo <= 0)
				sniperReloadTimer.Start();
			else
				if(sniper->Shoot(lastAim))
					state = PlayerState::SHOOTING_SNIPER;
		}
		else
		{
			state = PlayerState::RELOADING_SNIPER;
			if (sniperReloadTimer.ReadSec() >= sniperReloadTime)
			{
				sniper->Reload();
				sniperReloadTimer.Stop();
			}
		}
	}
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
	std::map<uint32, GameObject*>::const_iterator it = App->scene->GetGameObjects()->cbegin();

	for (it; it != App->scene->GetGameObjects()->cend(); ++it)
	{
		if (strstr(it->second->GetName(), "PrimaryWeapon") != nullptr)
		{
			ammoUi = it->second;
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
	std::map<uint32, GameObject*>::const_iterator it = App->scene->GetGameObjects()->cbegin();

	for (it; it != App->scene->GetGameObjects()->cend(); ++it)
	{
		if (strstr(it->second->GetName(), "Heart") != nullptr)
		{
			if (strstr(it->second->GetName(), "1") != nullptr)
			{
				hearts[0] = it->second;
			}
			else if (strstr(it->second->GetName(), "2") != nullptr)
			{
				hearts[1] = it->second;
			}
			else if (strstr(it->second->GetName(), "3") != nullptr)
			{
				hearts[2] = it->second;
			}
		}
	}

	if (full == nullptr)	{ full = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartFull.png"); }
	if (half == nullptr)	{ half = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartHalf.png"); }
	if (empty == nullptr)	{ empty = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/HUD/HeartEmpty.png"); }

	if (App->input->GetKey(SDL_SCANCODE_K) == KeyState::KEY_DOWN && hearts != nullptr)
	{
		health -= 0.5;

		if (health < 0)
			health = 0;

		if (health == 2.5)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(half);

		else if (health == 2)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(empty);

		else if (health == 1.5)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(half);

		else if (health == 1)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(empty);

		else if (health == 0.5)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(half);

		else if (health == 0)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(empty);
	}

	if (App->input->GetKey(SDL_SCANCODE_L) == KeyState::KEY_DOWN && hearts != nullptr)
	{

		if (health == 2.5)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(full);

		else if (health == 2)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(half);

		else if (health == 1.5)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(full);

		else if (health == 1)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(half);

		else if (health == 0.5)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(full);

		else if (health == 0)
			hearts[(int)health]->GetComponent<C_Material>()->SwapTexture(half);

		health += 0.5;

		if (health > maxHealth)
			health = maxHealth;
	}
}

Player* CreatePlayer()
{
	Player* script = new Player();

	// Character
	INSPECTOR_DRAGABLE_FLOAT(script->speed);
	INSPECTOR_DRAGABLE_INT(script->coins);

	// Dash
	INSPECTOR_DRAGABLE_FLOAT(script->dashSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingTime);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingColdown);

	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);
	INSPECTOR_DRAGABLE_FLOAT(script->invulnerability);

	// Weapons
	INSPECTOR_SLIDER_INT(script->weaponUsed, 1, 2);

	// Blaster
	INSPECTOR_PREFAB(script->blasterBullet);

	INSPECTOR_DRAGABLE_FLOAT(script->blasterSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->blasterRate);

	INSPECTOR_DRAGABLE_INT(script->blasterAmmo);
	INSPECTOR_DRAGABLE_INT(script->blasterMaxAmmo);
	INSPECTOR_DRAGABLE_FLOAT(script->blasterReloadTime);

	INSPECTOR_CHECKBOX_BOOL(script->blasterAutomatic);
	INSPECTOR_CHECKBOX_BOOL(script->strongShots);

	// Sniper
	INSPECTOR_PREFAB(script->sniperBullet);

	INSPECTOR_DRAGABLE_FLOAT(script->sniperSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->sniperRate);

	INSPECTOR_DRAGABLE_INT(script->sniperAmmo);
	INSPECTOR_DRAGABLE_INT(script->sniperMaxAmmo);
	INSPECTOR_DRAGABLE_FLOAT(script->sniperReloadTime);

	INSPECTOR_CHECKBOX_BOOL(script->sniperAutomatic);
	INSPECTOR_CHECKBOX_BOOL(script->freezingShots);

	// Animations
	INSPECTOR_STRING(script->idle);
	INSPECTOR_STRING(script->walk);
	INSPECTOR_STRING(script->dash);
	INSPECTOR_STRING(script->shootBlaster);
	INSPECTOR_STRING(script->shootSniper);
	INSPECTOR_STRING(script->die);

	return script;
}