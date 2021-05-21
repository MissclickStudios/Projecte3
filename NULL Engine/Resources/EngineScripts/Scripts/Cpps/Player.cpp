#include "JSONParser.h"

#include "Player.h"

#include "Application.h"
#include "M_Input.h"
#include "M_ResourceManager.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_Material.h"
#include "C_ParticleSystem.h"
#include "C_AudioSource.h"
#include "C_Mesh.h"

#include "C_2DAnimator.h"

#include "C_Animator.h"

#include "GameManager.h"

#include "Items.h"

#include "Log.h"

#define MAX_INPUT		32767
#define WALK_THRESHOLD	16384
#define WALKING_FACTOR	0.5f

Player* CreatePlayer()
{
	Player* script = new Player();

	// Entity ---
	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);

	// Basic Stats
	INSPECTOR_DRAGABLE_FLOAT(script->speed);
	INSPECTOR_DRAGABLE_FLOAT(script->attackSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->defense);

	// Modifiers
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealthModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->attackSpeedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->damageModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->defenseModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->cooldownModifier);

	// Death
	INSPECTOR_DRAGABLE_FLOAT(script->deathDuration);

	// Player ---
	// Walk
	INSPECTOR_DRAGABLE_FLOAT(script->walkSpeed);

	// Currency
	INSPECTOR_DRAGABLE_INT(script->currency);
	INSPECTOR_DRAGABLE_INT(script->hubCurrency);

	// Dash
	INSPECTOR_DRAGABLE_FLOAT(script->dashSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->dashDuration);
	INSPECTOR_DRAGABLE_FLOAT(script->dashCooldown);

	// Invencibility frames
	INSPECTOR_DRAGABLE_FLOAT(script->invincibilityDuration); 
	INSPECTOR_DRAGABLE_FLOAT(script->intermitentMesh);

	// Weapons
	INSPECTOR_DRAGABLE_FLOAT(script->changeTime);

	INSPECTOR_PREFAB(script->blaster);
	INSPECTOR_PREFAB(script->equipedGun);

	INSPECTOR_STRING(script->gameManager);

	//Hand Name
	INSPECTOR_STRING(script->handName);
	INSPECTOR_VECTOR_STRING(script->particleNames);

	//// Animations ---
	//// Movement
	//INSPECTOR_STRING(script->idleAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->idleAnimation.blendTime);
	//INSPECTOR_STRING(script->runAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->runAnimation.blendTime);
	//INSPECTOR_STRING(script->dashAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->dashAnimation.blendTime);
	//INSPECTOR_STRING(script->deathAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->deathAnimation.blendTime);
	//
	//// Aim
	//INSPECTOR_STRING(script->shootAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->shootAnimation.blendTime);
	//INSPECTOR_STRING(script->reloadAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->reloadAnimation.blendTime);
	//INSPECTOR_STRING(script->changeAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->changeAnimation.blendTime);
	//INSPECTOR_STRING(script->onGuardAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->onGuardAnimation.blendTime);

	return script;
}

Player::Player() : Entity()
{
	type = EntityType::PLAYER;
}

Player::~Player()
{
}

void Player::SetUp()
{
	dashTimer.Stop();
	dashCooldownTimer.Stop();
	invencibilityTimer.Stop();
	intermitentMeshTimer.Stop();
	changeTimer.Stop();

	// --- ANIMATIONS
	if (animator != nullptr)
	{
		torsoTrack	= animator->GetTrackAsPtr("Torso");
		legsTrack	= animator->GetTrackAsPtr("Legs");

		if (torsoTrack == nullptr)
			LOG("[ERROR] Player Script: Could not retrieve { TORSO } Animator Track! Error: C_Animator's GetTrackAsPtr() failed.");

		if (legsTrack == nullptr)
			LOG("[ERROR] Player Script: Could not retrieve { LEGS } Animator Track! Error: C_Animator's GetTrackAsPtr() failed.");
	}

	SetUpLegsMatrix();

	// --- RIGID BODY
	if (rigidBody != nullptr)
		rigidBody->TransformMovesRigidBody(false);

	// --- WEAPONS
	if (currentWeapon == nullptr)
		currentWeapon = blasterWeapon;

	// --- PARTICLES AND SFX
	runParticles	= GetParticles("Run");
	dashParticles	= GetParticles("Dash");

	(runParticles != nullptr)	? runParticles->StopSpawn()		: LOG("[ERROR] Player Script: Could not retrieve { RUN } Particle System! Error: GetParticles() failed.");
	(dashParticles != nullptr)	? dashParticles->StopSpawn()	: LOG("[ERROR] Player Script: Could not retrieve { DASH } Particle System! Error: GetParticles() failed.");

	for (uint i = 0; i < gameObject->components.size(); ++i)
	{
		if (gameObject->components[i]->GetType() == ComponentType::AUDIOSOURCE)
		{
			C_AudioSource* source = (C_AudioSource*)gameObject->components[i];
			std::string name = source->GetEventName();

			if (name == "mando_walking")		{ walkAudio = source; }
			else if (name == "mando_dash")		{ dashAudio = source; }
			else if (name == "weapon_change")	{ changeWeaponAudio = source; }
			else if (name == "mando_damaged")	{ damageAudio = source; }
			else if (name == "mando_death")		{ deathAudio = source; }
		}
	}

	idleAimPlane	= App->scene->GetGameObjectByName("IdlePlane");
	aimingAimPlane	= App->scene->GetGameObjectByName("AimingPlane");

	if (idleAimPlane == nullptr)
		LOG("COULD NOT RETRIEVE IDLE PLANE");
	else
		idleAimPlane->SetIsActive(true);

	if (aimingAimPlane == nullptr)
		LOG("COULD NOT RETRIEVE AIMING PLANE");
	else
		aimingAimPlane->SetIsActive(false);

	GameObject* a = App->scene->GetGameObjectByName(mandoImageName.c_str());
	if (a != nullptr)
		mandoImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(primaryWeaponImageName.c_str());
	if (a != nullptr)
		primaryWeaponImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(secondaryWeaponImageName.c_str());
	if (a != nullptr)
		secondaryWeaponImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(dashImageName.c_str());
	if (a != nullptr)
		dashImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(creditsImageName.c_str());
	if (a != nullptr)
		creditsImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();
}

void Player::Behavior()
{
	ManageMovement();
	if (moveState != PlayerState::DEAD || moveState != PlayerState::DEAD_OUT)
	{
		if (invencibilityTimer.IsActive())
		{
			if (invencibilityTimer.ReadSec() >= invincibilityDuration)
			{
				invencibilityTimer.Stop();
				intermitentMeshTimer.Stop();
				if (mesh != nullptr)
					mesh->SetIsActive(true);
			}
			else if (!intermitentMeshTimer.IsActive())
			{
				intermitentMeshTimer.Start();
				if (mesh != nullptr)
					mesh->SetIsActive(!mesh->IsActive());

				LOG("start int timer");
			}
			else if (intermitentMeshTimer.ReadSec() >= intermitentMesh)
			{
				intermitentMeshTimer.Stop();

				LOG("stop int timer");
			}
		}

		ManageAim();
	}
}

void Player::CleanUp()
{
	if (blasterGameObject != nullptr)
		blasterGameObject->toDelete = true;
	blasterGameObject = nullptr;
	blasterWeapon = nullptr;

	if (secondaryGunGameObject != nullptr)
		secondaryGunGameObject->toDelete = true;
	secondaryGunGameObject = nullptr;
	secondaryWeapon = nullptr;

	currentWeapon = nullptr;

	while (savedItems.size() != 0)
	{
		delete savedItems.begin()->second;
		savedItems.erase(savedItems.begin());
	}
}

void Player::EntityPause()
{
	dashTimer.Pause();
	dashCooldownTimer.Pause();
	invencibilityTimer.Pause();
	intermitentMeshTimer.Pause();
	changeTimer.Pause();
}

void Player::EntityResume()
{
	dashTimer.Resume();
	dashCooldownTimer.Resume();
	invencibilityTimer.Resume();
	intermitentMeshTimer.Resume();
	changeTimer.Resume();
}

void Player::SaveState(ParsonNode& playerNode)
{
	playerNode.SetInteger("Currency", currency);
	playerNode.SetInteger("Hub Currency", hubCurrency);

	playerNode.SetNumber("Health", health);

	playerNode.SetBool("God Mode", godMode);

	ParsonArray effectsArray = playerNode.SetArray("Effects");
	for (uint i = 0; i < effects.size(); ++i)
	{
		ParsonNode node = effectsArray.SetNode("Effect");
		node.SetInteger("Type", (int)effects[i]->Type());
		node.SetNumber("Duration", (double)effects[i]->RemainingDuration());
		node.SetBool("Permanent", effects[i]->Permanent());
	}

	playerNode.SetInteger("Equiped Gun", (int)equipedGun.uid);
	playerNode.SetBool("Using Equiped Gun", usingSecondaryGun);
	if (blasterWeapon != nullptr)
		playerNode.SetInteger("Blaster Ammo", blasterWeapon->ammo);

	if (secondaryWeapon != nullptr)
		playerNode.SetInteger("Equiped Gun Ammo", secondaryWeapon->ammo);

	ParsonArray itemArray = playerNode.SetArray("Items");
	for (uint i = 0; i < items.size(); ++i)
	{
		ParsonNode node = itemArray.SetNode("Item");
		node.SetBool("Weapon", items[i].first);
		node.SetString("Name", items[i].second->name.c_str());
		node.SetInteger("Rarity", (int)items[i].second->rarity);
		node.SetNumber("Power", items[i].second->power);
		node.SetNumber("Duration", items[i].second->duration);
		node.SetNumber("Chance", items[i].second->chance);
		node.SetString("Texture Path", items[i].second->texturePath.c_str());
	}
}

void Player::LoadState(ParsonNode& playerNode)
{
	currency = playerNode.GetInteger("Currency");
	hubCurrency = playerNode.GetInteger("Hub Currency");

	health = (float)playerNode.GetNumber("Health");

	godMode = playerNode.GetBool("God Mode");
	SetGodMode(godMode);

	ParsonArray effectsArray = playerNode.GetArray("Effects");
	for (uint i = 0; i < effectsArray.size; ++i)
	{
		ParsonNode node = effectsArray.GetNode(i);
		EffectType type = (EffectType)node.GetInteger("Type");
		float duration = (float)node.GetNumber("Duration");
		bool permanent = node.GetBool("Permanent");

		AddEffect(type, duration, permanent);
	}

	if (skeleton != nullptr)
		for (uint i = 0; i < skeleton->childs.size(); ++i)
		{
			std::string name = skeleton->childs[i]->GetName();
			if (name == "Hand")
			{
				hand = skeleton->childs[i];
				break;
			}
		}

	blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());
	if (blasterGameObject != nullptr)
	{
		blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);

		if (blasterWeapon != nullptr)
		{
			blasterWeapon->type = WeaponType::BLASTER;

			blasterWeapon->SetOwnership(type, hand, handName);
			blasterWeapon->ammo = playerNode.GetInteger("Blaster Ammo");
		}
	}

	// TODO: Load correct secondary gun
	//equipedGunGameObject = App->resourceManager->LoadPrefab(playerNode.GetInteger("Equiped Gun"), App->scene->GetSceneRoot());
	uint uid = (uint)playerNode.GetInteger("Equiped Gun");
	if (uid == NULL)
		uid = equipedGun.uid;
	secondaryGunGameObject = App->resourceManager->LoadPrefab(uid, App->scene->GetSceneRoot());
	if (secondaryGunGameObject != nullptr)
	{
		secondaryWeapon = (Weapon*)GetObjectScript(secondaryGunGameObject, ObjectType::WEAPON);

		if (secondaryWeapon != nullptr)
		{
			secondaryWeapon->SetOwnership(type, hand, handName);
			int savedAmmo = playerNode.GetInteger("Equiped Gun Ammo");
			if (savedAmmo > secondaryWeapon->MaxAmmo())
				savedAmmo = secondaryWeapon->MaxAmmo();
			secondaryWeapon->ammo = savedAmmo;
		}
	}

	ParsonArray itemArray = playerNode.GetArray("Items");
	for (uint i = 0; i < itemArray.size; ++i)
	{
		ParsonNode node = itemArray.GetNode(i);

		ItemData* savedData = new ItemData();
		bool usedWeapon = node.GetBool("Weapon");
		savedData->name = node.GetString("Name");
		savedData->rarity = (ItemRarity)node.GetInteger("Rarity");
		savedData->power = node.GetNumber("Power");
		savedData->duration = node.GetNumber("Duration");
		savedData->chance = node.GetNumber("Chance");
		savedData->texturePath = node.GetString("Texture Path");

		Item* item = Item::CreateItem(savedData); // Get an item with the data we loaded

		if (usedWeapon && secondaryWeapon != nullptr)
			currentWeapon = secondaryWeapon; // Set the current weapon to the one that has to recieve the item, fancy i know
		else
			currentWeapon = blasterWeapon;

		item->PickUp(this); // Apply the item effects and/or perks
		delete item;

		items.push_back(std::make_pair(usedWeapon, savedData));
		savedItems.push_back(std::make_pair(usedWeapon, savedData)); // Save this itemData to be deleted later
		// ye this is the best i can do, deal with it
		// any brilliant solutions u might have to this problem weren't aparent at the time
		// so fix it if u want but dont bother me
	}

	usingSecondaryGun = playerNode.GetBool("Using Equiped Gun");
	if (usingSecondaryGun && secondaryWeapon != nullptr)
	{
		currentWeapon = secondaryWeapon;
		if (blasterWeapon->weaponModel != nullptr)
			blasterWeapon->weaponModel->SetIsActive(false);
		if (secondaryWeapon->weaponModel != nullptr)
			secondaryWeapon->weaponModel->SetIsActive(true);
	}
	else
	{
		currentWeapon = blasterWeapon;
		if (blasterWeapon->weaponModel != nullptr)
			blasterWeapon->weaponModel->SetIsActive(true);
		if (secondaryWeapon->weaponModel != nullptr)
			secondaryWeapon->weaponModel->SetIsActive(false);
	}
}

void Player::Reset()
{
	currency = 0;

	health = maxHealth;

	while (effects.size())
	{
		delete* effects.begin();
		effects.erase(effects.begin());
	}

	if (blasterWeapon != nullptr)
	{
		blasterWeapon->ammo = blasterWeapon->maxAmmo;
		blasterWeapon->perks.clear();
	}
	if (secondaryWeapon != nullptr)
	{
		secondaryWeapon->ammo = secondaryWeapon->maxAmmo;
		secondaryWeapon->perks.clear();
	}

	items.clear();
	while (savedItems.size() != 0)
	{
		delete savedItems.begin()->second;
		savedItems.erase(savedItems.begin());
	}

	usingSecondaryGun = false;
}

void Player::TakeDamage(float damage)
{
	if (!invencibilityTimer.IsActive())
	{
		float damageDealt = 0.0f;
		if(Defense())
		 damageDealt = damage / Defense();
		health -= damageDealt;

		if (health < 0.0f)
			health = 0.0f;
		invencibilityTimer.Start();

		hitTimer.Start();
		if (GetParticles("Hit") != nullptr)
			GetParticles("Hit")->ResumeSpawn();
		if (material != nullptr)
		{
			material->SetAlternateColour(Color(1, 0, 0, 1));
			material->SetTakeDamage(true);
		}

		if (damageAudio != nullptr)
			damageAudio->PlayFx(damageAudio->GetEventId());
	}
}


void Player::EquipWeapon(Prefab weapon)
{
	if (secondaryWeapon != nullptr)
	{
		if (secondaryWeapon->weaponModel != nullptr)
			secondaryWeapon->weaponModel->SetIsActive(false);
		secondaryWeapon = nullptr;
	}
	if (secondaryGunGameObject != nullptr)
	{
		secondaryGunGameObject->toDelete = true;
		secondaryGunGameObject = nullptr;
	}

	secondaryGunGameObject = App->resourceManager->LoadPrefab(weapon.uid, App->scene->GetSceneRoot());
	equipedGun = weapon;
	if (secondaryGunGameObject != nullptr)
	{
		secondaryWeapon = (Weapon*)GetObjectScript(secondaryGunGameObject, ObjectType::WEAPON);

		if (secondaryWeapon != nullptr)
		{
			secondaryWeapon->SetOwnership(type, hand, handName);
			currentWeapon = secondaryWeapon;
		}
	}
}

void Player::SetPlayerInteraction(InteractionType type)
{
	switch (type)
	{
	case InteractionType::NONE:				{;} break;
	case InteractionType::TALK:				{;} break;
	case InteractionType::USE:				{;} break;
	case InteractionType::OPEN_CHEST:		{;} break;
	case InteractionType::SIGNAL_GROGU:		{;} break;
	}

	currentInteraction = type;
}

void Player::AnimatePlayer()
{
	if (animator == nullptr)
		return;

	AnimatorTrack* preview = animator->GetTrackAsPtr("Preview");

	if (aimState == AimState::IDLE)
	{	
		if (torsoTrack != nullptr)
		{
			if (torsoTrack->GetTrackState() != TrackState::STOP)
				torsoTrack->Stop();
		}
			
		if (legsTrack != nullptr)
		{
			if (legsTrack->GetTrackState() != TrackState::STOP)
				legsTrack->Stop();
		}
		
		AnimatorClip* previewClip = preview->GetCurrentClip();

		if ((previewClip == nullptr) || (previewClip->GetName() != currentAnimation->name))											// If no clip playing or animation/clip changed
			animator->PlayClip(currentAnimation->track.c_str(), currentAnimation->name.c_str(), currentAnimation->blendTime);

		if (preview->GetTrackState() == TrackState::STOP)
		{
			preview->Play();
		}
	}
	else
	{	
		LOG("DIRECTIONS: [%d]::[%d]::[%s]", aimDirection, moveDirection, GetLegsAnimation()->name.c_str());
		
		AnimationInfo* torsoInfo	= GetAimStateAnimation();
		AnimationInfo* legsInfo		= GetMoveStateAnimation();
		if (torsoInfo == nullptr || legsInfo == nullptr)
		{
			LOG("DEFAULTING AIMING TO PREVIEW");
			
			AnimatorClip* previewClip = preview->GetCurrentClip();

			if ((previewClip == nullptr) || (previewClip->GetName() != currentAnimation->name))										// If no clip playing or animation/clip changed
				animator->PlayClip(currentAnimation->track.c_str(), currentAnimation->name.c_str(), currentAnimation->blendTime);
		}
		else
		{
			if (torsoTrack == nullptr || legsTrack == nullptr)
			{
				LOG("[WARNING] Player Script: torsoTrack or legsTrack was nullptr!");
				return;
			}

			if (preview->GetTrackState() != TrackState::STOP)
				preview->Stop();

			AnimatorClip* torsoClip = torsoTrack->GetCurrentClip();
			AnimatorClip* legsClip	= legsTrack->GetCurrentClip();

			//LOG("TORSO: [%s]::[%s]::[%s]", torsoInfo->track.c_str(), torsoInfo->name.c_str(), (torsoClip != nullptr) ? torsoClip->GetName() : "NO TORSO CLIP");
			//LOG("LEGS: [%s]::[%s]::[%s]", legsInfo->track.c_str(), legsInfo->name.c_str(), (legsClip != nullptr) ? legsClip->GetName() : "NO LEGS CLIP");

			if ((torsoClip == nullptr) || (torsoClip->GetName() != torsoInfo->name))
				animator->PlayClip(torsoTrack->GetName(), torsoInfo->name.c_str(), torsoInfo->blendTime);

			if ((legsClip == nullptr) || (legsClip->GetName() != legsInfo->name))
				animator->PlayClip(legsTrack->GetName(), legsInfo->name.c_str(), legsInfo->blendTime);

			if (torsoTrack->GetTrackState() == TrackState::STOP)
				torsoTrack->Play();

			if (legsTrack->GetTrackState() == TrackState::STOP)
				legsTrack->Play();
		}
	}
}

AnimationInfo* Player::GetMoveStateAnimation()
{
	switch (moveState)
	{
	case PlayerState::IDLE:		{ return &idleAnimation; }		break;
	case PlayerState::WALK:		{ return &walkAnimation; }		break;
	case PlayerState::RUN:		{ return /*GetLegsAnimation();*/ &runAnimation; }	break;
	case PlayerState::DASH_IN:	{ return &dashAnimation; }		break;
	case PlayerState::DASH:		{ return &dashAnimation; }		break;
	case PlayerState::DEAD_IN:	{ return &deathAnimation; }		break;
	case PlayerState::DEAD:		{ return &deathAnimation; }		break;
	}
	
	return nullptr;
}

AnimationInfo* Player::GetLegsAnimation()
{	
	if (moveInput.IsZero())
		return &idleAnimation;

	return legsMatrix[(int)aimDirection][(int)moveDirection];
}

AnimationInfo* Player::GetAimStateAnimation()
{
	switch (aimState)
	{
	case AimState::IDLE:		{ return &idleAnimation; }			break;
	case AimState::ON_GUARD:	{ return &idleAnimation; }			break;
	case AimState::AIMING:		{ return GetAimAnimation(); }		break;
	case AimState::SHOOT_IN:	{ return GetShootAnimation(); }		break;
	case AimState::SHOOT:		{ return GetShootAnimation(); }		break;
	case AimState::RELOAD_IN:	{ return GetReloadAnimation(); }	break;
	case AimState::RELOAD:		{ return GetReloadAnimation(); }	break;
	case AimState::CHANGE_IN:	{ return &changeWeaponAnimation; }	break;
	case AimState::CHANGE:		{ return &changeWeaponAnimation; }	break;
	}
	
	return nullptr;
}

AnimationInfo* Player::GetAimAnimation()
{
	if (currentWeapon == nullptr)
		return nullptr;
	
	switch (currentWeapon->type)
	{
	case WeaponType::BLASTER:	{ /*LOG("AIM BLASTER{ %s }", aimBlasterAnimation.name.c_str());*/	return &aimBlasterAnimation; }	break;
	case WeaponType::SNIPER:	{ /*LOG("AIM SNIPER { %s }", aimSniperAnimation.name.c_str());*/	return &aimSniperAnimation; }	break;
	case WeaponType::SHOTGUN:	{ /*LOG("AIM SHOTGUN{ %s }", aimShotgunAnimation.name.c_str());*/	return &aimShotgunAnimation; }	break;
	case WeaponType::MINIGUN:	{ /*LOG("AIM MINIGUN{ %s }", aimMinigunAnimation.name.c_str());*/	return &aimMinigunAnimation; }	break;
	}
	
	LOG("COULD NOT GET AIM ANIMATION");

	return nullptr;
}

AnimationInfo* Player::GetShootAnimation()
{
	if (currentWeapon == nullptr)
		return nullptr;
	
	switch (currentWeapon->type)
	{
	case WeaponType::BLASTER:	{ /*LOG("SHOOT BLASTER { %s }", shootBlasterAnimation.name.c_str());*/	return &shootBlasterAnimation; }	break;
	case WeaponType::SNIPER:	{ /*LOG("SHOOT SNIPER  { %s }", shootSniperAnimation.name.c_str());*/	return &shootSniperAnimation; }		break;
	case WeaponType::SHOTGUN:	{ /*LOG("SHOOT SHOTGUN { %s }", shootShotgunAnimation.name.c_str());*/	return &shootShotgunAnimation; }	break;
	case WeaponType::MINIGUN:	{ /*LOG("SHOOT MINIGUN { %s }", shootMinigunAnimation.name.c_str());*/	return &shootMinigunAnimation; }	break;
	}

	LOG("COULD NOT GET SHOOT ANIMATION");

	return nullptr;
}

AnimationInfo* Player::GetReloadAnimation()
{
	if (currentWeapon == nullptr)
		return nullptr;

	switch (currentWeapon->type)
	{
	case WeaponType::BLASTER:	{ /*LOG("RELOAD BLASTER { %s }", reloadBlasterAnimation.name.c_str());*/	return &reloadBlasterAnimation; }	break;
	case WeaponType::SNIPER:	{ /*LOG("RELOAD SNIPER  { %s }", reloadSniperAnimation.name.c_str());*/		return &reloadSniperAnimation; }	break;
	case WeaponType::SHOTGUN:	{ /*LOG("RELOAD SHOTGUN { %s }", reloadShotgunAnimation.name.c_str());*/	return &reloadShotgunAnimation; }	break;
	case WeaponType::MINIGUN:	{ /*LOG("RELOAD MINIGUN { %s }", reloadMinigunAnimation.name.c_str());*/	return &reloadMinigunAnimation; }	break;
	}
	
	LOG("COULD NOT GET RELOAD ANIMATION");

	return nullptr;
}

void Player::SetGodMode(bool enable)
{
	godMode = enable;
	defense = (godMode) ? 0.0f : 1.0f;
}

bool Player::GetGodMode() const
{
	return godMode;
}

void Player::SetUpLegsMatrix()
{
	// All NONE elements will most probably not be used. Added just in case.
	legsMatrix[(int)AimDirection::NONE][(int)MoveDirection::NONE]			= &idleAnimation;					// AN + MN --> I
	legsMatrix[(int)AimDirection::NONE][(int)MoveDirection::FORWARDS]		= &runForwardsAnimation;			// AN + MF --> F
	legsMatrix[(int)AimDirection::NONE][(int)MoveDirection::BACKWARDS]		= &runBackwardsAnimation;			// AN + MB --> B
	legsMatrix[(int)AimDirection::NONE][(int)MoveDirection::LEFT]			= &runLeftAnimation;				// AN + ML --> L
	legsMatrix[(int)AimDirection::NONE][(int)MoveDirection::RIGHT]			= &runRightAnimation;				// AN + MR --> R

	legsMatrix[(int)AimDirection::FORWARDS][(int)MoveDirection::NONE]		= &idleAnimation;					// AF + MN --> I
	legsMatrix[(int)AimDirection::FORWARDS][(int)MoveDirection::FORWARDS]	= &runForwardsAnimation;			// AF + MF --> F
	legsMatrix[(int)AimDirection::FORWARDS][(int)MoveDirection::BACKWARDS]	= &runBackwardsAnimation;			// AF + MB --> B
	legsMatrix[(int)AimDirection::FORWARDS][(int)MoveDirection::LEFT]		= &runLeftAnimation;				// AF + ML --> L
	legsMatrix[(int)AimDirection::FORWARDS][(int)MoveDirection::RIGHT]		= &runRightAnimation;				// AF + MR --> R

	legsMatrix[(int)AimDirection::BACKWARDS][(int)MoveDirection::NONE]		= &idleAnimation;					// AB + MN --> I
	legsMatrix[(int)AimDirection::BACKWARDS][(int)MoveDirection::FORWARDS]	= &runBackwardsAnimation;			// AB + MF --> B
	legsMatrix[(int)AimDirection::BACKWARDS][(int)MoveDirection::BACKWARDS]	= &runForwardsAnimation;			// AB + MB --> F
	legsMatrix[(int)AimDirection::BACKWARDS][(int)MoveDirection::LEFT]		= &runRightAnimation;				// AB + ML --> R
	legsMatrix[(int)AimDirection::BACKWARDS][(int)MoveDirection::RIGHT]		= &runLeftAnimation;				// AB + MR --> L

	legsMatrix[(int)AimDirection::LEFT][(int)MoveDirection::NONE]			= &idleAnimation;					// AL + MN --> I
	legsMatrix[(int)AimDirection::LEFT][(int)MoveDirection::FORWARDS]		= &runRightAnimation;				// AL + MF --> R
	legsMatrix[(int)AimDirection::LEFT][(int)MoveDirection::BACKWARDS]		= &runLeftAnimation;				// AL + MB --> L
	legsMatrix[(int)AimDirection::LEFT][(int)MoveDirection::LEFT]			= &runForwardsAnimation;			// AL + ML --> F
	legsMatrix[(int)AimDirection::LEFT][(int)MoveDirection::RIGHT]			= &runBackwardsAnimation;			// AL + MR --> B

	legsMatrix[(int)AimDirection::RIGHT][(int)MoveDirection::NONE]			= &runLeftAnimation;				// AR + MN --> I
	legsMatrix[(int)AimDirection::RIGHT][(int)MoveDirection::FORWARDS]		= &runLeftAnimation;				// AR + MF --> L
	legsMatrix[(int)AimDirection::RIGHT][(int)MoveDirection::BACKWARDS]		= &runRightAnimation;				// AR + MB --> R
	legsMatrix[(int)AimDirection::RIGHT][(int)MoveDirection::LEFT]			= &runBackwardsAnimation;			// AR + ML --> B
	legsMatrix[(int)AimDirection::RIGHT][(int)MoveDirection::RIGHT]			= &runForwardsAnimation;			// AR + MR --> F
}

void Player::AddItem(ItemData* item)
{
	for (uint i = 0; i < items.size(); ++i)
	{
		if (items[i].first == usingSecondaryGun && items[i].second->name == item->name && items[i].second->rarity <= item->rarity)
		{
			items[i].second = item;
			ApplyItems();
			return;
		}
	}
	items.push_back(std::make_pair(usingSecondaryGun, item));
}

void Player::ApplyItems()
{
	if (currentWeapon == nullptr)
		return;

	currentWeapon->RefreshPerks(true);
	Item* item = nullptr;
	for (uint i = 0; i < items.size(); ++i)
	{
		item = Item::CreateItem(items[i].second);
		item->PickUp(this);
		delete item;
	}
}

void Player::ManageMovement()
{
	if (moveState != PlayerState::DEAD || moveState != PlayerState::DEAD_OUT)
	{
		if (health <= 0.0f)
		{
			moveState = PlayerState::DEAD_IN;
			//moveState = PlayerState::DEAD;
		}
		else
		{
			GatherInteractionInputs();

			if (currentInteraction == InteractionType::NONE)
			{
				if (moveState != PlayerState::DASH)
					GatherMoveInputs();
			}
		}
	}

	if (runParticles != nullptr)
		runParticles->StopSpawn();

	if (rigidBody == nullptr)														// Ending early in case there is no rigidBody associated with the player.
		return;

	switch (moveState)
	{
	case PlayerState::IDLE:		{ MoveIdle(); } break;
	case PlayerState::INTERACT: { Interact(); } break;
	case PlayerState::WALK:		{ Walk(); }		break;
	case PlayerState::RUN:		{ Run(); }		break;

	case PlayerState::DASH_IN:	{ DashIn(); }
	case PlayerState::DASH:		{ Dash(); }		break;

	case PlayerState::DEAD_IN:	{ DeadIn(); }
	case PlayerState::DEAD:		{ Dead(); }		break;
	}
}

void Player::ManageAim()
{
	if (moveState != PlayerState::DASH)
		GatherAimInputs();

	Aim();

	switch (aimState)
	{
	case AimState::IDLE:		{ AimIdle(); }	break;
	case AimState::ON_GUARD:	{ OnGuard(); }	break;
	case AimState::AIMING:		{ Aiming(); }	break;

	case AimState::SHOOT_IN:	{ ShootIn(); }
	case AimState::SHOOT:		{ Shoot(); }	break;

	case AimState::RELOAD_IN:	{ ReloadIn(); }
	case AimState::RELOAD:		{ Reload(); }	break;

	case AimState::CHANGE_IN:	{ ChangeIn(); }
	case AimState::CHANGE:		{ Change(); }	break;
	}
}

// --- MOVEMENT STATES
void Player::MoveIdle()
{
	currentAnimation = &idleAnimation;

	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(float2::zero);
}

void Player::Interact()
{

}

void Player::Walk()
{
	currentAnimation = &walkAnimation;
	Movement();

	if (runParticles != nullptr)
		runParticles->ResumeSpawn();
}

void Player::Run()
{
	currentAnimation = &runAnimation;
	Movement();

	if (runParticles != nullptr)
		runParticles->ResumeSpawn();

}

void Player::DashIn()
{
	if (dashAudio != nullptr)
		dashAudio->PlayFx(dashAudio->GetEventId());

	currentAnimation = &dashAnimation;

	dashTimer.Start();

	if (rigidBody != nullptr)
	{
		rigidBody->ChangeFilter(" player dashing");
		rigidBody->FreezePositionY(true);
	}

	if (dashParticles != nullptr)
		dashParticles->ResumeSpawn();

	moveState = PlayerState::DASH;
}

void Player::Dash()
{
	ApplyDash();

	if (dashTimer.ReadSec() >= DashDuration())				// When the dash duration ends start the cooldown and reset the move state
	{
		dashTimer.Stop();
		dashCooldownTimer.Start();

		if (rigidBody != nullptr)
		{
			rigidBody->ChangeFilter(" player");
			rigidBody->FreezePositionY(false);
		}

		if (dashParticles != nullptr)
			dashParticles->StopSpawn();

		moveState = PlayerState::IDLE;
	}
}

void Player::DeadIn()
{
	currentAnimation = &deathAnimation;

	if (deathAudio != nullptr)
		deathAudio->PlayFx(deathAudio->GetEventId());

	if (rigidBody != nullptr)
		rigidBody->SetIsActive(false);						// Disable the rigidbody to avoid more interactions with other entities

	deathTimer.Start();

	moveState = PlayerState::DEAD;
}

void Player::Dead()
{
	if (deathTimer.ReadSec() >= deathDuration)
		moveState = PlayerState::DEAD_OUT;
}

// --- AIM STATES
void Player::AimIdle()
{
	/*if (idleAimPlane != nullptr)
		idleAimPlane->SetIsActive(true);*/

	if (aimingAimPlane != nullptr)
		aimingAimPlane->SetIsActive(false);
}

void Player::OnGuard()
{
	aimState = AimState::IDLE;
}

void Player::Aiming()
{
	/*if (idleAimPlane != nullptr)
		idleAimPlane->SetIsActive(false);*/

	if (aimingAimPlane != nullptr)
		aimingAimPlane->SetIsActive(true);
}

void Player::ShootIn()
{
	currentAnimation	= GetShootAnimation();
	aimState			= AimState::SHOOT;
}

void Player::Shoot()
{
	if (currentWeapon == nullptr)
		return;
	
	currentAnimation = GetShootAnimation();

	switch (currentWeapon->Shoot(aimVector))
	{
	case ShootState::NO_FULLAUTO:		{ currentAnimation = nullptr; aimState = AimState::ON_GUARD; }		break;
	case ShootState::WAITING_FOR_NEXT:	{ /* DO NOTHING */ }												break;
	case ShootState::FIRED_PROJECTILE:	{ currentAnimation = nullptr; aimState = AimState::ON_GUARD; }		break;
	case ShootState::RATE_FINISHED:		{ currentAnimation = nullptr; aimState = AimState::ON_GUARD; }		break;
	case ShootState::NO_AMMO:			{ /*currentAnimation = nullptr;*/ aimState = AimState::RELOAD_IN; }	break;
	}
}

void Player::ReloadIn()
{
	aimState = AimState::RELOAD;
}

void Player::Reload()
{
	if (currentWeapon != nullptr && currentWeapon->Reload())
		aimState = AimState::ON_GUARD;
}

void Player::ChangeIn()
{
	changeTimer.Start();
	
	if (changeWeaponAudio != nullptr)
		changeWeaponAudio->PlayFx(changeWeaponAudio->GetEventId());

	aimState = AimState::CHANGE;
}

void Player::Change()
{
	if (changeTimer.ReadSec() < ChangeTime())
		return;
	
	if (blasterWeapon == currentWeapon)
	{
		usingSecondaryGun	= true;
		currentWeapon		= secondaryWeapon;

		if (blasterWeapon->weaponModel != nullptr)
			blasterWeapon->weaponModel->SetIsActive(false);

		if (secondaryWeapon->weaponModel != nullptr)
			secondaryWeapon->weaponModel->SetIsActive(true);
	}
	else
	{
		usingSecondaryGun	= false;
		currentWeapon		= blasterWeapon;

		if (blasterWeapon->weaponModel != nullptr)
			blasterWeapon->weaponModel->SetIsActive(true);

		if (secondaryWeapon->weaponModel != nullptr)
			secondaryWeapon->weaponModel->SetIsActive(false);
	}

	aimState = AimState::ON_GUARD;
}

void Player::GatherMoveInputs()
{
	// Controller movement
	moveInput.x = (float)App->input->GetGameControllerAxisValue(0);
	moveInput.y = (float)App->input->GetGameControllerAxisValue(1);

	// Keyboard movement
	if (moveInput.IsZero())	// If there was no controller input
	{
		if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT) { moveInput.y = -MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT) { moveInput.y = MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT) { moveInput.x = MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT) { moveInput.x = -MAX_INPUT; }
	}

	SetPlayerDirection();

	if (!dashCooldownTimer.IsActive())
	{
		if ((App->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_DOWN || App->input->GetGameControllerTrigger(0) == ButtonState::BUTTON_DOWN))
		{
			if (!dashTimer.IsActive())
				moveState = PlayerState::DASH_IN;
				//moveState = PlayerState::DASH;

			return;
		}
	}
	else if (dashCooldownTimer.ReadSec() >= DashCooldown())
	{
		dashCooldownTimer.Stop();
	}

	if (!moveInput.IsZero())
	{	
		bool overWalkThreshold = (moveInput.x > WALK_THRESHOLD) || (-moveInput.x > WALK_THRESHOLD) || (moveInput.y > WALK_THRESHOLD) || (-moveInput.y > WALK_THRESHOLD);
		
		moveState = (overWalkThreshold) ? PlayerState::RUN : PlayerState::WALK;

		return;
	}

	moveState = PlayerState::IDLE;
}

void Player::GatherAimInputs()
{
	// Controller aim
	aimInput.x = (float)App->input->GetGameControllerAxisValue(2);
	aimInput.y = (float)App->input->GetGameControllerAxisValue(3);
	
	// Keyboard aim
	if (aimInput.IsZero())	// If there was no controller input
	{
		if (App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_REPEAT)	{ aimInput.y = -MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_REPEAT)	{ aimInput.y = MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_REPEAT)	{ aimInput.x = MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_REPEAT)	{ aimInput.x = -MAX_INPUT; }
	}

	SetAimDirection();

	if (aimState != AimState::IDLE && aimState != AimState::AIMING && aimState != AimState::ON_GUARD) // If the player is not on this states, ignore action inputs (shoot, reload, etc.)
		return;

	if (App->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN)
	{
		aimState = AimState::CHANGE_IN;
		//aimState = AimState::CHANGE;
		return;
	}

	if ((App->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN))
	{
		aimState = AimState::RELOAD_IN;
		//aimState = AimState::RELOAD;
		return;
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_REPEAT || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_REPEAT)
	{
		aimState = AimState::SHOOT_IN;
		return;
	}

	aimState = AimState::IDLE;
}

void Player::GatherInteractionInputs()
{

}

void Player::SetPlayerDirection()
{
	if (moveInput.IsZero())
	{
		moveDirection = MoveDirection::NONE;
		return;
	}

	float absX = moveInput.x * moveInput.x;
	float absY = moveInput.y * moveInput.y;

	if (absX > absY)
	{
		moveDirection = (moveInput.x < 0.0f) ? MoveDirection::LEFT : MoveDirection::RIGHT;
	}
	else
	{
		moveDirection = (moveInput.y < 0.0f) ? MoveDirection::FORWARDS : MoveDirection::BACKWARDS;
	}
}

void Player::SetAimDirection()
{
	if (aimInput.IsZero())
	{
		aimDirection = AimDirection::NONE;
		return;
	}

	float absX = aimInput.x * aimInput.x;
	float absY = aimInput.y * aimInput.y;

	if (absX > absY)
	{
		aimDirection = (aimInput.x < 0.0f) ? AimDirection::LEFT : AimDirection::RIGHT;
	}
	else
	{
		aimDirection = (aimInput.y < 0.0f) ? AimDirection::FORWARDS : AimDirection::BACKWARDS;
	}
}

void Player::Movement()
{
	moveVector = moveInput;

	float speed = (moveState == PlayerState::RUN) ? Speed() : (walkSpeed * speedModifier);

	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity((moveInput.Normalized()) * speed);
}

void Player::Aim()
{	
	if (aimState == AimState::IDLE && !aimInput.IsZero())
	{
		aimState = AimState::AIMING;
	}
	if (aimState == AimState::AIMING && aimInput.IsZero())
	{
		aimState = AimState::IDLE;
	}

	aimVector = (aimInput.IsZero() || moveState == PlayerState::DASH) ? moveVector : aimInput;

	float rad = aimVector.AimedAngle();
	
	if (skeleton != nullptr)
		skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
}

void Player::ApplyDash()
{	
	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity((moveVector.Normalized()) * DashSpeed());
	
	//dashImage->PlayAnimation(false, 1);
}