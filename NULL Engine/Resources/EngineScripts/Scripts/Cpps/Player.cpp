#include "JSONParser.h"

#include "Player.h"

#include "VariableTypedefs.h"
#include "Log.h"

#include "Application.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"
#include "M_Camera3D.h"
#include "M_Window.h"
#include "M_Audio.h"
#include "M_Physics.h"
#include "M_FileSystem.h"

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

#include "LevelGenerator.h"
#include "Weapon.h"

#include "MC_Time.h"

#include "MathGeoLib/include/Geometry/Line.h"

#define MAX_JOYSTICK_INPUT 32767
#define MANDO_FILE "EngineScripts/Mando.json"

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
	char* buffer = nullptr;
	uint size = App->fileSystem->Load(MANDO_FILE, &buffer);
	if (buffer)
	{
		ParsonNode config(buffer);
	
		weaponUsed = config.GetInteger("Weapon used");
		coins = config.GetInteger("Coins");
		strongShots = true; //config.GetBool("Strong shots");
		freezingShots = true; //config.GetBool("Freezing shots");
		blasterAmmo = config.GetInteger("Blaster ammo");
		sniperAmmo = config.GetInteger("Sniper ammo");
		health = maxHealth; //(float)config.GetNumber("Health");

		load = true;
	}

	RELEASE_ARRAY(buffer);

	if (!gameObject->GetComponent<C_RigidBody>())
		gameObject->CreateComponent(ComponentType::RIGIDBODY);

	std::vector<Component*> components;
	gameObject->GetAllComponents(components);
	for (auto comp = components.begin(); comp != components.end(); ++comp)
	{
		if ((*comp)->GetType() == ComponentType::AUDIOSOURCE)
		{
			C_AudioSource* source = (C_AudioSource*)(*comp);
			std::string name = source->GetEventName();

			if (name == "mando_damaged")
				damaged = source;
			else if (name == "mando_dash")
				dashSource = source;
			else if (name == "mando_walking")
				step = source;
			else if (name == "mando_death")
				death = source;
			else if (name == "weapon_reload_01")
				reloadBlaster = source;
			else if (name == "weapon_reload_02")
				reloadSniper = source;
			else if (name == "weapon_change")
				weaponChange = source;
		}
	}

	if (!blaster)
		blaster = new Weapon(gameObject, blasterBullet, 10u, blasterMaxAmmo, blasterSpeed, blasterRate, blasterAutomatic);
	if (!sniper)
		sniper = new Weapon(gameObject, sniperBullet, 2u, sniperMaxAmmo, sniperSpeed, sniperRate, sniperAutomatic);



	std::vector<GameObject*>* gameObjects = App->scene->GetGameObjects();
	for (auto object = gameObjects->begin(); object != gameObjects->end(); ++object)
	{
		std::string name = (*object)->GetName();
		if (name == "Blaster")
			blasterModel = (*object);
		else if (name == "Sniper")
			sniperModel = (*object);
	}

	if (weaponUsed == 1)
	{
		if (blasterModel)
			blasterModel->SetIsActive(true);
		if (sniperModel)
			sniperModel->SetIsActive(false);
	}
	else
	{
		if (blasterModel)
			blasterModel->SetIsActive(false);
		if (sniperModel)
			sniperModel->SetIsActive(true);
	}

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
		if (state != PlayerState::DEAD)
		{
			state = PlayerState::DEAD;

			weaponUsed = 1;
			coins = 0;
			strongShots = false;
			freezingShots = false;
			health = maxHealth;
			blasterAmmo = blasterMaxAmmo;
			sniperAmmo = sniperMaxAmmo;

			if (death)
				death->PlayFx(death->GetEventId());

			C_RigidBody* rigidBody = gameObject->GetComponent<C_RigidBody>();
			if (!rigidBody || rigidBody->IsStatic())
				rigidBody->SetIsActive(false);

			//TODO: Player death
			//App->scene->GetLevelGenerator()->InitiateLevel(1);
		}
		return;
	}

	Movement();

	blaster->Update();
	sniper->Update();
	if (load)
	{
		load = false;
		if (strongShots)
			BlasterStrongShots();
		if (freezingShots)
			SniperFreezingShots();

		blaster->ammo = blasterAmmo;
		sniper->ammo = sniperAmmo;
	}

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
	ParsonNode config;
	config.SetInteger("Weapon used", weaponUsed);
	config.SetInteger("Coins", coins);
	config.SetInteger("Blaster ammo", blasterAmmo);
	config.SetInteger("Sniper ammo", sniperAmmo);
	config.SetNumber("Health", (double)health);
	config.SetBool("Strong shots", strongShots);
	config.SetBool("Freezing shots", freezingShots);

	char* buffer = nullptr;
	config.SerializeToFile(MANDO_FILE, &buffer);

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
		if (damaged)
			damaged->PlayFx(damaged->GetEventId());

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
	if (blaster)
		for (uint i = 0; i < blaster->projectilesNum; ++i)
			((Bullet*)blaster->projectiles[i]->object->GetScript("Bullet"))->strong = true;
}

void Player::SniperFreezingShots()
{
	if (sniper)
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
		{
			aAnimator->PlayClip(idle, 0.2f);
		}
		break;
	case PlayerState::RELOADING_SNIPER:
	//	if (currentClip != nullptr && clipName != "Shooting")
	//	{
	//		//aAnimator->PlayClip("Reloading", 0u);
	//	}
		if (currentClip && clipName != idle)
		{
			aAnimator->PlayClip(idle, 0.2f);
		}
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

			if (dashSource)
				dashSource->PlayFx(dashSource->GetEventId());
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

			if (weaponChange)
				weaponChange->PlayFx(weaponChange->GetEventId());
		}
		else
		{
			weaponUsed = 1;
			if (blasterModel)
				blasterModel->SetIsActive(true);
			if (sniperModel)
				sniperModel->SetIsActive(false);

			if (weaponChange)
				weaponChange->PlayFx(weaponChange->GetEventId());
		}

	bool shooting = false;
	if (weaponUsed == 1)
	{
		if (!blasterReloadTimer.IsActive())
		{
			if ((App->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN)
				|| blasterAmmo <= 0)
			{
				blasterReloadTimer.Start();
				if (reloadBlaster)
					reloadBlaster->PlayFx(reloadBlaster->GetEventId());
			}
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
			{
				sniperReloadTimer.Start();
				if (reloadSniper)
					reloadSniper->PlayFx(reloadSniper->GetEventId());
			}
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
		
		if (step)
			step->PlayFx(step->GetEventId());
	}
	else
		if (stepTimer.ReadSec() >= 0.4f)
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
	std::vector<GameObject*>::const_iterator it = App->scene->GetGameObjects()->cbegin();

	for (it; it != App->scene->GetGameObjects()->cend(); ++it)
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
	std::vector<GameObject*>::const_iterator it = App->scene->GetGameObjects()->cbegin();

	for (it; it != App->scene->GetGameObjects()->cend(); ++it)
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

	if (full == nullptr)	{ full = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/UI/HUD/HeartFull.png"); }
	if (half == nullptr)	{ half = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/UI/HUD/HeartHalf.png"); }
	if (empty == nullptr)	{ empty = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/UI/HUD/HeartEmpty.png"); }

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