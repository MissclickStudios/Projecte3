#include "MathGeoLib/include/Math/float3.h"
#include "JSONParser.h"

#include "Log.h"

#include "Application.h"
#include "M_Input.h"
#include "M_ResourceManager.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Material.h"
#include "C_RigidBody.h"
#include "C_ParticleSystem.h"
#include "C_AudioSource.h"
#include "C_2DAnimator.h"
#include "C_Animator.h"

#include "GameManager.h"

#include "Items.h"

#include "Player.h"

#define MAX_INPUT				32767.0f
#define WALK_THRESHOLD			16384.0f
#define AIM_THRESHOLD			8192.0f
#define P_JOYSTICK_THRESHOLD	4096.0f
#define WALKING_FACTOR			0.5f

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
	INSPECTOR_DRAGABLE_FLOAT(script->aimingSpeed);

	// Currency
	INSPECTOR_DRAGABLE_INT(script->credits);
	INSPECTOR_DRAGABLE_INT(script->beskar);

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

	// Hand Name
	INSPECTOR_STRING(script->rightHandName);
	INSPECTOR_STRING(script->leftHandName);

	// Aim
	INSPECTOR_STRING(script->idleAimPlaneName);
	INSPECTOR_STRING(script->aimingAimPlaneName);

	// Animations
	INSPECTOR_STRING(script->hipName);
	INSPECTOR_STRING(script->torsoName);
	INSPECTOR_STRING(script->legsName);

	// Particles & SFX
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
	// --- TIMERS
	dashTimer.Stop();
	dashCooldownTimer.Stop();
	invincibilityTimer.Stop();
	intermitentMeshTimer.Stop();
	changeTimer.Stop();
	interactionTimer.Stop();

	// --- SCENES
	inHub = !strcmp(App->scene->GetCurrentScene(), "HUB");

	// --- GAME OBJECTS
	rightHand		= gameObject->FindChild(rightHandName.c_str());
	leftHand		= gameObject->FindChild(leftHandName.c_str());
	idleAimPlane	= gameObject->FindChild(idleAimPlaneName.c_str());
	aimingAimPlane	= gameObject->FindChild(aimingAimPlaneName.c_str());
	hip				= gameObject->FindChild(hipName.c_str());
	torso			= gameObject->FindChild(torsoName.c_str());
	legs			= gameObject->FindChild(legsName.c_str());

	if (rightHand == nullptr)		{ LOG("[ERROR] Player Script: Could not retrieve { %s }! Error: GameObject* FindChild() failed.", rightHandName.c_str()); }
	if (leftHand == nullptr)		{ LOG("[ERROR] Player Script: Could not retrieve { %s }! Error: GameObject* FindChild() failed.", leftHandName.c_str()); }
	if (idleAimPlane == nullptr)	{ LOG("[ERROR] Player Script: Could not retrieve { %s }! Error: GameObject* FindChild() failed.", idleAimPlaneName.c_str()); }
	if (aimingAimPlane == nullptr)	{ LOG("[ERROR] Player Script: Could not retrieve { %s }! Error: GameObject* FindChild() failed.", aimingAimPlaneName.c_str()); }
	if (hip == nullptr)				{ LOG("[ERROR] Player Script: Could not retrieve { %s }! Error: GameObject* FindChild() failed.", hipName.c_str()); }
	if (torso == nullptr)			{ LOG("[ERROR] Player Script: Could not retrieve { %s }! Error: GameObject* FindChild() failed.", torsoName.c_str()); }
	if (legs == nullptr)			{ LOG("[ERROR] Player Script: Could not retrieve { %s }! Error: GameObject* FindChild() failed.", legsName.c_str()); }

	// --- ANIMATIONS
	if (animator != nullptr)
	{
		torsoTrack	= animator->GetTrackAsPtr("Torso");
		legsTrack	= animator->GetTrackAsPtr("Legs");
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

	// UI ELEMENTS
	(idleAimPlane != nullptr)	? idleAimPlane->SetIsActive(true)		: LOG("COULD NOT RETRIEVE IDLE PLANE");
	(aimingAimPlane != nullptr) ? aimingAimPlane->SetIsActive(false)	: LOG("COULD NOT RETRIEVE AIMING PLANE");

	GameObject* uiImageGO	= App->scene->GetGameObjectByName(mandoImageName.c_str());
	mandoImage				= (uiImageGO != nullptr) ? (C_2DAnimator*)uiImageGO->GetComponent<C_2DAnimator>() : nullptr;		// If the GO was found, get the C_2DAnimator*.

	uiImageGO				= App->scene->GetGameObjectByName(primaryWeaponImageName.c_str());
	primaryWeaponImage		= (uiImageGO != nullptr) ? (C_2DAnimator*)uiImageGO->GetComponent<C_2DAnimator>() : nullptr;

	uiImageGO				= App->scene->GetGameObjectByName(secondaryWeaponImageName.c_str());
	secondaryWeaponImage	= (uiImageGO != nullptr) ? (C_2DAnimator*)uiImageGO->GetComponent<C_2DAnimator>() : nullptr;

	uiImageGO				= App->scene->GetGameObjectByName(dashImageName.c_str());
	dashImage				= (uiImageGO != nullptr) ? (C_2DAnimator*)uiImageGO->GetComponent<C_2DAnimator>() : nullptr;

	uiImageGO				= App->scene->GetGameObjectByName(creditsImageName.c_str());
	creditsImage			= (uiImageGO != nullptr) ? (C_2DAnimator*)uiImageGO->GetComponent<C_2DAnimator>() : nullptr;

	uiImageGO				= App->scene->GetGameObjectByName(beskarImageName.c_str());
	beskarImage				= (uiImageGO != nullptr) ? (C_2DAnimator*)uiImageGO->GetComponent<C_2DAnimator>() : nullptr;
}

void Player::Behavior()
{	
	if (!allowInput)
		return;

	usingKeyboard		= App->input->KeyboardReceivedInputs();
	usingGameController = App->input->GameControllerReceivedInputs();

	ManageInteractions();

	if (currentInteraction == InteractionType::NONE)																			// All inputs will be ignored while the player is interacting.
	{
		ManageMovement();

		if (moveState != PlayerState::DEAD && moveState != PlayerState::DEAD_OUT)
		{
			ManageAim();

			ManageInvincibility();

			doDieCutscene = false;
		}
		else
		{
			doDieCutscene = true;
		}
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
	invincibilityTimer.Pause();
	intermitentMeshTimer.Pause();
	changeTimer.Pause();
}

void Player::EntityResume()
{
	dashTimer.Resume();
	dashCooldownTimer.Resume();
	invincibilityTimer.Resume();
	intermitentMeshTimer.Resume();
	changeTimer.Resume();
}

void Player::SaveState(ParsonNode& playerNode)
{
	playerNode.SetInteger("Currency", credits);
	playerNode.SetInteger("Hub Currency", beskar);

	playerNode.SetNumber("Health", health);

	playerNode.SetBool("God Mode", godMode);

	ParsonArray effectsArray = playerNode.SetArray("Effects");
	for (uint i = 0; i < effects.size(); ++i)
	{
		ParsonNode node = effectsArray.SetNode("Effect");
		node.SetInteger("Type", (int)effects[i]->Type());
		node.SetNumber("Duration", (double)effects[i]->RemainingDuration());
		node.SetNumber("Power", effects[i]->Power());
		node.SetNumber("Chance", effects[i]->Chance());
		node.SetNumber("DirectionX", effects[i]->Direction().x);
		node.SetNumber("DirectionY", effects[i]->Direction().y);
		node.SetNumber("DirectionZ", effects[i]->Direction().z);
		node.SetBool("Permanent", effects[i]->Permanent());
		node.SetBool("Start", effects[i]->start);
	}

	playerNode.SetInteger("Equiped Gun", (int)equipedGun.uid);
	LOG("Saved Equiped gun uid : %d", equipedGun.uid);
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
	credits = playerNode.GetInteger("Currency");
	beskar = playerNode.GetInteger("Hub Currency");

	health = (float)playerNode.GetNumber("Health");

	godMode = playerNode.GetBool("God Mode");
	SetGodMode(godMode);

	ParsonArray effectsArray = playerNode.GetArray("Effects");
	for (uint i = 0; i < effectsArray.size; ++i)
	{
		ParsonNode node = effectsArray.GetNode(i);
		EffectType type = (EffectType)node.GetInteger("Type");
		float duration = (float)node.GetNumber("Duration");
		float power = (float)node.GetNumber("Power");
		float chance = (float)node.GetNumber("Chance");
		float3 direction;
		direction.x = (float)node.GetNumber("DirectionX");
		direction.y = (float)node.GetNumber("DirectionY");
		direction.z = (float)node.GetNumber("DirectionZ");
		bool permanent = node.GetBool("Permanent");
		bool start = node.GetBool("Start");

		AddEffect(type, duration, permanent, power, chance, direction, start);
	}

	if (skeleton != nullptr)
		for (uint i = 0; i < skeleton->childs.size(); ++i)
		{
			std::string name = skeleton->childs[i]->GetName();
			if (name == "RightHand")
			{
				rightHand = skeleton->childs[i];
				break;
			}
			if (name == "LeftHand")
			{
				leftHand = skeleton->childs[i];
			}
		}

	blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());
	if (blasterGameObject != nullptr)
	{
		blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);

		if (blasterWeapon != nullptr)
		{
			blasterWeapon->type = WeaponType::BLASTER;

			blasterWeapon->SetOwnership(type, rightHand, rightHandName.c_str());
			blasterWeapon->ammo = playerNode.GetInteger("Blaster Ammo");
		}
	}

	// TODO: Load correct secondary gun
	//equipedGunGameObject = App->resourceManager->LoadPrefab(playerNode.GetInteger("Equiped Gun"), App->scene->GetSceneRoot());
	uint uid = (uint)playerNode.GetInteger("Equiped Gun");
	LOG("Equiped gun loaded uid : %d", uid);
	if (uid == NULL)
		uid = equipedGun.uid;
	secondaryGunGameObject = App->resourceManager->LoadPrefab(uid, App->scene->GetSceneRoot());
	if (secondaryGunGameObject != nullptr)
	{
		equipedGun.uid = uid;
		secondaryWeapon = (Weapon*)GetObjectScript(secondaryGunGameObject, ObjectType::WEAPON);

		if (secondaryWeapon != nullptr)
		{
			secondaryWeapon->SetOwnership(type, rightHand, rightHandName.c_str());
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
	credits = 0;

	health = MaxHealth();

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

	GameObject* object = App->scene->GetGameObjectByName(gameManager.c_str());
	if (object != nullptr)
	{
		GameManager* manager = (GameManager*)object->GetScript("GameManager");
		if (manager != nullptr)
		{
			std::vector<ItemData*> hubItems = manager->GetHubItemPool();
			if (manager->armorLvl)
			{
				ItemData* const itemData = Item::FindItem(hubItems, "Durasteel Reinforcement", (ItemRarity)manager->armorLvl);
				if (itemData != nullptr)
				{
					Item::CreateItem(itemData)->PickUp(this);
					AddItem(itemData); // yes you have to manually add the item after picking it up, I KNOW... its kinda wack
				}
			}
			if (manager->bootsLvl)
			{
				ItemData* const itemData = Item::FindItem(hubItems, "Propulsed Boots", (ItemRarity)manager->bootsLvl);
				if (itemData != nullptr)
				{
					Item::CreateItem(itemData)->PickUp(this);
					AddItem(itemData);
				}
			}
			if (manager->ticketLvl)
			{
				ItemData* const itemData = Item::FindItem(hubItems, "Premium Ticket", (ItemRarity)manager->ticketLvl);
				if (itemData != nullptr)
				{
					Item::CreateItem(itemData)->PickUp(this);
					AddItem(itemData);
				}
			}
			if (manager->bottleLvl)
			{
				ItemData* const itemData = Item::FindItem(hubItems, "Refrigeration Liquid", (ItemRarity)manager->bottleLvl);
				if (itemData != nullptr)
				{
					Item::CreateItem(itemData)->PickUp(this);
					AddItem(itemData);
				}
			}
		}
	}
}

void Player::EnableInput()
{
	allowInput = true;
}

void Player::DisableInput()
{
	allowInput = false;
}

void Player::TakeDamage(float damage)
{
	if (!invincibilityTimer.IsActive())
	{
		float damageDealt = 0.0f;
		if(Defense())
		 damageDealt = 1.0f; // heehee
		health -= damageDealt;

		if (health < 0.0f)
			health = 0.0f;
		invincibilityTimer.Start();

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

	secondaryGunGameObject	= App->resourceManager->LoadPrefab(weapon.uid, App->scene->GetSceneRoot());
	equipedGun				= weapon;
	if (secondaryGunGameObject != nullptr)
	{
		secondaryWeapon = (Weapon*)GetObjectScript(secondaryGunGameObject, ObjectType::WEAPON);

		if (secondaryWeapon != nullptr)
		{
			if (secondaryWeapon->type == WeaponType::MINIGUN)
			{
				secondaryWeapon->SetOwnership(type, leftHand, leftHandName.c_str());
				currentWeapon = secondaryWeapon;
			}
			else
			{
				secondaryWeapon->SetOwnership(type, rightHand, rightHandName.c_str());
				currentWeapon = secondaryWeapon;
			}
		}
	}
}

void Player::GiveCredits(int _credits)
{
	credits += _credits;
	//play HUD credits animation
	creditsImage->PlayAnimation(false, 1);
}

void Player::GiveBeskar(int _beskar)
{
	beskar += _beskar;
	//play HUD beskar animation
	beskarImage->PlayAnimation(false, 1);
}

void Player::SubtractCredits(int _credits)
{
	credits -= _credits;
	//play HUD credits animation
	creditsImage->PlayAnimation(false, 1);
}

void Player::SubtractBeskar(int _beskar)
{
	beskar -= _beskar;
	//play HUD credits animation
	beskarImage->PlayAnimation(false, 1);
}

void Player::SetPlayerInteraction(InteractionType type, float duration)
{	
	currentInteraction	= type;
	interactionDuration = duration;

	if (type == InteractionType::NONE)
	{
		interactionTimer.Stop();
		interactionDuration = 0.0f;

		if (rigidBody != nullptr)																				// Making the player dynamic again once the interaction has finished.
			rigidBody->MakeDynamic();

		if (dashTimer.IsActive())																				// In case the interaction was set while the player was dashing.
			moveState = PlayerState::DASH;

		return;
	}

	moveState	= PlayerState::IDLE;
	aimState	= AimState::IDLE;
	
	if (rigidBody != nullptr)																					// Making sure that the player will remain still while interacting.
		rigidBody->MakeStatic();

	switch (currentInteraction)
	{
	case InteractionType::USE:				{ Use(); }			break;
	case InteractionType::BUY:				{ Buy(); }			break;
	case InteractionType::TALK:				{ Talk(); }			break;
	case InteractionType::OPEN_CHEST:		{ OpenChest(); }	break;
	case InteractionType::SIGNAL_GROGU:		{ SignalGrogu(); }	break;
	}
}

void Player::AnimatePlayer()
{	
	if (animator == nullptr)
		return;
	
	AnimatorTrack* preview		= animator->GetTrackAsPtr("Preview");
	AnimationInfo* torsoInfo	= GetAimStateAnimation();
	AnimationInfo* legsInfo		= GetMoveStateAnimation();

	//LOG("CURRENT:	{ %s }",	(currentAnimation != nullptr) ? currentAnimation->name.c_str() : "NONE");
	//LOG("TORSO:	  { %s }",		(torsoInfo != nullptr) ? torsoInfo->name.c_str() : "NONE");
	//LOG("LEGS:	   { %s }",		(legsInfo != nullptr) ? legsInfo->name.c_str() : "NONE");

	if (GetEntityState() != EntityState::NONE || aimState == AimState::IDLE || torsoInfo == nullptr || legsInfo == nullptr)		// TAKE INTO ACCOUNT STUN AND KNOCKBACK + LOOK INTO DASH PROBLEMS 
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
		//LOG("DIRECTIONS: [%d]::[%d]::[%s]::[%s]", aimDirection, moveDirection, GetAimStateAnimation()->name.c_str(), GetLegsAnimation()->name.c_str());

		if (torsoTrack == nullptr || legsTrack == nullptr)
		{
			LOG("[WARNING] Player Script: torsoTrack or legsTrack was nullptr!");
			return;
		}

		(hip != nullptr) ? hip->transform->SetLocalRotation(float3::zero) : LOG("OOGA BOOGA NO HIPAROOGA");			// Resetting the hip position.
			
		AnimatorClip* torsoClip = torsoTrack->GetCurrentClip();
		AnimatorClip* legsClip = legsTrack->GetCurrentClip();

		if (preview->GetTrackState() != TrackState::STOP)
			preview->Stop();

		if ((torsoClip == nullptr) || (torsoClip->GetName() != torsoInfo->name))
		{
			animator->PlayClip(torsoTrack->GetName(), torsoInfo->name.c_str(), torsoInfo->blendTime);

			/*if (torsoTrack->GetTrackState() == TrackState::STOP)
			{
				torsoTrack->Play();
			}*/
		}

		if ((legsClip == nullptr) || (legsClip->GetName() != legsInfo->name))
		{
			animator->PlayClip(legsTrack->GetName(), legsInfo->name.c_str(), legsInfo->blendTime);

			/*if (legsTrack->GetTrackState() == TrackState::STOP)
			{
				legsTrack->Play();
			}*/
		}
	}
}

AnimationInfo* Player::GetMoveStateAnimation()
{
	switch (moveState)
	{
	case PlayerState::IDLE:		{ return &idleAnimation; }		break;
	case PlayerState::WALK:		{ return &walkAnimation; }		break;
	case PlayerState::RUN:		{ return GetLegsAnimation(); }	break;
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

	if (aimInput.IsZero())
	{
		if (currentWeapon == nullptr)
			return &runForwardsLightAnimation;

		return (currentWeapon->type != WeaponType::MINIGUN) ? &runForwardsLightAnimation : &runForwardsHeavyAnimation;
	}

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
	case WeaponType::BLASTER:	{ return &aimBlasterAnimation; }	break;
	case WeaponType::SNIPER:	{ return &aimSniperAnimation; }		break;
	case WeaponType::SHOTGUN:	{ return &aimShotgunAnimation; }	break;
	case WeaponType::MINIGUN:	{ return &aimMinigunAnimation; }	break;
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
	case WeaponType::BLASTER:	{ return &shootBlasterAnimation; }	break;
	case WeaponType::SNIPER:	{ return &shootSniperAnimation; }	break;
	case WeaponType::SHOTGUN:	{ return &shootShotgunAnimation; }	break;
	case WeaponType::MINIGUN:	{ return &shootMinigunAnimation; }	break;
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
	case WeaponType::BLASTER:	{ return &reloadBlasterAnimation; }	break;
	case WeaponType::SNIPER:	{ return &reloadSniperAnimation; }	break;
	case WeaponType::SHOTGUN:	{ return &reloadShotgunAnimation; }	break;
	case WeaponType::MINIGUN:	{ return &reloadMinigunAnimation; }	break;
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
		if (items[i].second->name == item->name && items[i].second->rarity <= item->rarity)
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
	if (blasterWeapon != nullptr)
		blasterWeapon->RefreshPerks(true);
	if (secondaryWeapon != nullptr)
		secondaryWeapon->RefreshPerks(true);

	Item* item = nullptr;
	for (uint i = 0; i < items.size(); ++i)
	{
		item = Item::CreateItem(items[i].second);
		item->PickUp(this);
		delete item;
	}
}

float Player::GetAnimatorClipDuration(const char* clipName)
{
	AnimatorClip* clip = animator->GetClipAsPtr(clipName);

	return (clip != nullptr) ? clip->GetDurationInSeconds() : 0.0f;
}

void Player::ManageInteractions()
{
	GatherInteractionInputs();

	if (currentInteraction == InteractionType::TALK)															// If the interaction is TALK, then it'll be ended through the dialog system.
		return;

	if (currentInteraction != InteractionType::NONE && !interactionTimer.IsActive())							// If there is an interaction and the timer is not active.
	{
		interactionTimer.Start();
	}
	else if (currentInteraction == InteractionType::NONE && interactionTimer.IsActive())						// If there is no interaction and the timer is active.
	{
		interactionTimer.Stop();
		interactionDuration = 0.0f;
	}

	if (interactionTimer.ReadSec() > interactionDuration)
	{
		interactionTimer.Stop();
		SetPlayerInteraction(InteractionType::NONE);
	}
}

void Player::ManageMovement()
{	
	if (moveState != PlayerState::DEAD && moveState != PlayerState::DEAD_OUT)
	{
		if (health <= 0.0f)
		{
			moveState = PlayerState::DEAD_IN;
			aimState = AimState::IDLE;
		}
		else
		{
			if (moveState != PlayerState::DASH)
				GatherMoveInputs();
		}
	}

	if (runParticles != nullptr)
		runParticles->StopSpawn();

	if (rigidBody == nullptr)																					// Ending early in case there is no rigidBody associated with the player.
		return;

	switch (moveState)
	{
	case PlayerState::IDLE:		{ MovementIdle(); } break;
	case PlayerState::WALK:		{ Walk(); }			break;
	case PlayerState::RUN:		{ Run(); }			break;

	case PlayerState::DASH_IN:	{ DashIn(); }																	// All "_IN" states will be used to Start their main counterparts.
	case PlayerState::DASH:		{ Dash(); }			break;

	case PlayerState::DEAD_IN:	{ DeadIn(); }
	case PlayerState::DEAD:		{ Dead(); }			break;
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

void Player::ManageInvincibility()
{	
	if (!invincibilityTimer.IsActive())
		return;
	
	if (invincibilityTimer.ReadSec() >= invincibilityDuration)
	{
		invincibilityTimer.Stop();
		intermitentMeshTimer.Stop();
		
		if (mesh != nullptr)
			mesh->SetIsActive(true);
	}
	else if (!intermitentMeshTimer.IsActive())
	{
		intermitentMeshTimer.Start();
		
		if (mesh != nullptr)
			mesh->SetIsActive(!mesh->IsActive());

		//LOG("start int timer");
	}
	else if (intermitentMeshTimer.ReadSec() >= intermitentMesh)
	{
		intermitentMeshTimer.Stop();

		//LOG("stop int timer");
	}
}

// --- INTERACTION STATE METHODS
void Player::Talk()
{
	currentAnimation = &talkAnimation;

	if (interactionDuration == 0.0f)
		interactionDuration = GetAnimatorClipDuration("Talk");
}

void Player::Use()
{
	currentAnimation = &useAnimation;

	if (interactionDuration == 0.0f)
		interactionDuration = GetAnimatorClipDuration("Use");
}

void Player::Buy()
{
	currentAnimation = &useAnimation;

	if (interactionDuration == 0.0f)
		interactionDuration = GetAnimatorClipDuration("Use");
}

void Player::OpenChest()
{
	currentAnimation = &openChestAnimation;

	if (interactionDuration == 0.0f)
		interactionDuration = GetAnimatorClipDuration("OpenChest");
}

void Player::SignalGrogu()
{
	currentAnimation = &signalGroguAnimation;

	if (interactionDuration == 0.0f)
		interactionDuration = GetAnimatorClipDuration("SignalGrogu");
}

// --- MOVEMENT STATE METHODS
void Player::MovementIdle()
{
	currentAnimation = &idleAnimation;

	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(float2::zero);
}

void Player::Walk()
{
	currentAnimation = &walkAnimation;
	
	Movement();

	if (!inHub && runParticles != nullptr)
		runParticles->ResumeSpawn();
}

void Player::Run()
{
	currentAnimation = &runForwardsAnimation;
	
	Movement();

	if (!inHub && runParticles != nullptr)
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

// --- AIM STATE METHODS
void Player::AimIdle()
{
	if (currentWeapon != nullptr)
	{
		currentWeapon->defPosition = currentWeapon->position;
		currentWeapon->defRotation = currentWeapon->rotation;
	}
}

void Player::OnGuard()
{
	aimState = AimState::IDLE;
}

void Player::Aiming()
{
	currentAnimation = GetAimAnimation();
	
	if (currentWeapon != nullptr)
	{
		currentWeapon->defPosition = currentWeapon->modifiedPosition;
		currentWeapon->defRotation = currentWeapon->modifiedRotation;
	}
}

void Player::ShootIn()
{
	currentAnimation	= GetShootAnimation();
	aimState			= AimState::SHOOT;
	primaryWeaponImage->PlayAnimation(false, 1);
}

void Player::Shoot()
{
	if (currentWeapon == nullptr)
		return;
	
	//currentAnimation = GetShootAnimation();

	switch (currentWeapon->Shoot(aimVector))
	{
	case ShootState::NO_FULLAUTO:		{ currentAnimation = nullptr; aimState = AimState::IDLE; }			break;
	case ShootState::WAITING_FOR_NEXT:	{ /* DO NOTHING */ }												break;
	case ShootState::FIRED_PROJECTILE:	
	{ 
		if (currentWeapon->type != WeaponType::MINIGUN)
		{ 
			currentAnimation = nullptr; 
			aimState = AimState::IDLE;
		}
		else
		{
			currentWeapon->defPosition = currentWeapon->modifiedPosition;
			currentWeapon->defRotation = currentWeapon->modifiedRotation;
		}
	}	break;
	case ShootState::RATE_FINISHED:		{ currentAnimation = nullptr; aimState = AimState::IDLE; }			break;
	case ShootState::NO_AMMO:			{ /*currentAnimation = nullptr;*/ aimState = AimState::RELOAD_IN; }	break;
	}
}

void Player::ReloadIn()
{
	LOG("RELOADIN");
	
	currentAnimation = GetReloadAnimation();

	aimState = AimState::RELOAD;
	primaryWeaponImage->PlayAnimation(false, 3);
}

void Player::Reload()
{
	if (currentWeapon != nullptr && currentWeapon->Reload())	
		aimState = AimState::ON_GUARD;
}

void Player::ChangeIn()
{
	changeTimer.Start();
	
	currentAnimation = &changeWeaponAnimation;

	if (changeWeaponAudio != nullptr)
		changeWeaponAudio->PlayFx(changeWeaponAudio->GetEventId());

	aimState = AimState::CHANGE;
}

void Player::Change()
{
	LOG("Change Weapon Time %.3f", ChangeTime());
	
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

	primaryWeaponImage->PlayAnimation(false, 2);

	aimState = AimState::ON_GUARD;
}

void Player::GatherMoveInputs()
{
	// Controller movement
	moveInput.x = (float)App->input->GetGameControllerAxisInput(LEFT_JOYSTICK_X_AXIS);														// Gets the axis input regardless of thresholds
	moveInput.y = (float)App->input->GetGameControllerAxisInput(LEFT_JOYSTICK_Y_AXIS);														// and axis states. Careful with joystick noise.

	//LOG("[Keyboard: %s]::[Controller: %s]", (usingKeyboard) ? "True" : "False", (usingGameController) ? "True" : "False");
	//LOG("MOVE INPUT: { %.3f, %.3f }", moveInput.x, moveInput.y);

	if (!dashCooldownTimer.IsActive())
	{
		if ((App->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_DOWN || App->input->GetGameControllerTrigger(LEFT_TRIGGER) == ButtonState::BUTTON_DOWN))
		{
			if (!dashTimer.IsActive())
				moveState = PlayerState::DASH_IN;

			return;
		}
	}
	else if (dashCooldownTimer.ReadSec() >= DashCooldown())
	{
		dashCooldownTimer.Stop();
	}

	if (usingKeyboard && !usingGameController)																								// If there was keyboard input and no GC input.
	{	
		if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT)	{ moveInput.y = -MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)	{ moveInput.y = MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)	{ moveInput.x = MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)	{ moveInput.x = -MAX_INPUT; }

		if (abs(moveInput.x) >= MAX_INPUT || abs(moveInput.y) >= MAX_INPUT)
		{
			moveState = (App->input->GetKey(SDL_SCANCODE_LCTRL) != KeyState::KEY_REPEAT) ? PlayerState::RUN : PlayerState::WALK;
		}
		else
		{
			moveState = PlayerState::IDLE;
		}

		//moveState = (abs(moveInput.x) >= MAX_INPUT || abs(moveInput.y) >= MAX_INPUT) ? PlayerState::RUN : PlayerState::IDLE;
	}
	else if (usingGameController)																											// If there was controller input.
	{	
		if (abs(moveInput.x) > P_JOYSTICK_THRESHOLD || abs(moveInput.y) > P_JOYSTICK_THRESHOLD)
		{
			moveState = (abs(moveInput.x) > WALK_THRESHOLD || abs(moveInput.y) > WALK_THRESHOLD) ? PlayerState::RUN : PlayerState::WALK;
		}
		else
		{
			moveState = PlayerState::IDLE;
		}
	}
	else
	{
		if (moveState != PlayerState::DASH && moveState != PlayerState::DEAD)
			moveState = PlayerState::IDLE;
	}

	SetPlayerDirection();
}

void Player::GatherAimInputs()
{
	// Controller aim
	aimInput.x = (float)App->input->GetGameControllerAxisRaw(RIGHT_JOYSTICK_X_AXIS);							// x right joystick
	aimInput.y = (float)App->input->GetGameControllerAxisRaw(RIGHT_JOYSTICK_Y_AXIS);							// y right joystick

	aimInputThreshold.x = (float)App->input->GetGameControllerAxisValue(RIGHT_JOYSTICK_X_AXIS);					// x right joystick with threshhold
	aimInputThreshold.y = (float)App->input->GetGameControllerAxisValue(RIGHT_JOYSTICK_Y_AXIS);					// y right joystick with threshhold

	//LOG("AIM INPUT		--> [%.3f]::[%.3f]", aimInput.x, aimInput.y);
	//LOG("AIM THRESHOLD	--> [%.3f]::[%.3f]", aimInputThreshold.x, aimInputThreshold.y);

	if (aimState != AimState::IDLE && aimState != AimState::AIMING && aimState != AimState::ON_GUARD)			// If the player is in this states, ignore action inputs (shoot, reload, etc.)
	{
		// aimState = AimState::IDLE;
		return;
	}
	
	// Keyboard aim
	if (usingKeyboard && !usingGameController)																	// If there was no controller input
	{
		if (App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_REPEAT)	{ aimInput.y = -MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_REPEAT)	{ aimInput.y = MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_REPEAT)	{ aimInput.x = MAX_INPUT; }
		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_REPEAT)	{ aimInput.x = -MAX_INPUT; }
		
		if (abs(aimInput.x) < MAX_INPUT && abs(aimInput.y) < MAX_INPUT)
		{
			aimState = AimState::IDLE;
		}
		else
		{
			if (aimState == AimState::IDLE)
				aimState = AimState::AIMING;
		}

	}
	else if (usingGameController)																														//There is input above the threshold
	{
		if (abs(aimInputThreshold.x) <= AIM_THRESHOLD && abs(aimInputThreshold.y) <= AIM_THRESHOLD)
		{
			aimState = AimState::IDLE;
		}
		else
		{
			if (aimState == AimState::IDLE)
				aimState = AimState::AIMING;
		}
		
		//aimState = (abs(aimInputThreshold.x) <= AIM_THRESHOLD && abs(aimInputThreshold.y) <= AIM_THRESHOLD) ? AimState::IDLE : AimState::AIMING;
	}
	else
	{
		aimState = AimState::IDLE;
	}

	SetAimDirection();

	if (App->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(SDL_CONTROLLER_BUTTON_B) == ButtonState::BUTTON_DOWN)
	{
		aimState = AimState::CHANGE_IN;
		return;
	}

	if ((App->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(SDL_CONTROLLER_BUTTON_X) == ButtonState::BUTTON_DOWN))
	{
		aimState = AimState::RELOAD_IN;
		return;
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_REPEAT || App->input->GetGameControllerTrigger(RIGHT_TRIGGER) == ButtonState::BUTTON_REPEAT)
	{
		aimState = AimState::SHOOT_IN;
		return;
	}
}

void Player::GatherInteractionInputs()
{
	if (health <= 0.0f)
	{
		SetPlayerInteraction(InteractionType::NONE);
		return;
	}
	
	if (currentInteraction == InteractionType::NONE)																					// For debug purposes.
	{
		if (App->input->GetKey(SDL_SCANCODE_RCTRL) == KeyState::KEY_REPEAT)
		{
			LOG("ACCESSING DEBUGGE MODDE");
			
			if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_DOWN) { SetPlayerInteraction(InteractionType::TALK); }
			if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_DOWN) { SetPlayerInteraction(InteractionType::USE); }
			if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_DOWN) { SetPlayerInteraction(InteractionType::OPEN_CHEST); }
			if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_DOWN) { SetPlayerInteraction(InteractionType::SIGNAL_GROGU); }
		}
	}
	
	if (currentInteraction == InteractionType::TALK)																					// Bring the controller TALK finisher here too.
	{
		if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KeyState::KEY_DOWN)
		{
			SetPlayerInteraction(InteractionType::NONE);
		}
	}
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
	if (aimState == AimState::IDLE)
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
	moveVector	= moveInput;																		// This method will only be called if moveState == PlayerState::WALK || PlayerState::RUN.
	float speed = Speed();

	if (moveState == PlayerState::WALK)
		speed = (walkSpeed * speedModifier);

	if (aimState == AimState::SHOOT)
		speed = (aimingSpeed * speedModifier);

	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity((moveInput.Normalized()) * speed);
}

void Player::Aim()
{
	float2 oldAim = aimVector;

	if (aimState == AimState::IDLE || moveState == PlayerState::DASH)												// AimState::IDLE means not aiming
	{	
		aimVector = (!moveVector.IsZero()) ? moveVector : oldAim;
	}
	else
	{
		aimVector = (abs(aimInput.x) < AIM_THRESHOLD && abs( aimInput.y) < AIM_THRESHOLD) ? moveVector : aimInput;

		//if (abs(aimInput.x) < AIM_THRESHOLD && abs(aimInput.y) < AIM_THRESHOLD)
		//	LOG("USING MOVE VECTOR: moveVector --> { %.3f, %.3f } || aimInput --> { %.3f, %.3f }", moveVector.x, moveVector.y, aimInput.x, aimInput.y);
	}

	if (abs(aimInput.x) < WALK_THRESHOLD && abs(aimInput.y) < WALK_THRESHOLD)										// Only works with this specific threshold. If it works it works.
	{
		if (aimingAimPlane != nullptr)
			aimingAimPlane->SetIsActive(false);
	}
	else
	{
		if (aimingAimPlane != nullptr)
			aimingAimPlane->SetIsActive(true);
	}

	float rad = aimVector.AimedAngle();

	if (skeleton != nullptr)
		skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
	
	/*if (abs(aimVector.x) >= P_JOYSTICK_THRESHOLD || abs(aimVector.y) >= P_JOYSTICK_THRESHOLD)					// In case the rotation has to be locked under the JOYSTICK_THRESHOLD.
	{
		if (skeleton != nullptr)
			skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
	}*/

	//LOG("Aim vector x = %f , y = %f", aimVector.x, aimVector.y);
}

void Player::ApplyDash()
{	
	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity((moveVector.Normalized()) * DashSpeed());
	
	//dashImage->PlayAnimation(false, 1);
}