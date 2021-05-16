#include "Player.h"
#include "Weapon.h"

#include "ItemRarity.h"

#include "JSONParser.h"

#include <string>

#define EFFECTS_STATS_ARRAY_NAME "Effects Stats"

// Data that acuratelly stores all information of an item stored in an item pool
struct ItemData
{
	ItemData() : name(""), description(""), price(0), rarity(ItemRarity::COMMON), power(0.0f), duration(0.0f), chance(0.0f), min(0), max(0), texturePath("") {}
	ItemData(std::string name, std::string description, int price, ItemRarity rarity, float power, float duration, float chance, int min, int max, std::string texturePath)
		: name(name), description(description), price(price), rarity(rarity), power(power), duration(duration), chance(chance), min(min), max(max), texturePath(texturePath) {}

	std::string name;
	std::string description;
	int price;
	ItemRarity rarity;

	float power;
	float duration;
	float chance;

	int min;
	int max;

	std::string texturePath;
};

class Item
{
public:

	Item(const ItemData* const itemData, bool toBuy)
	{
		name = itemData->name;
		description = itemData->description;

		if (toBuy)
			price = itemData->price;
		else
			price = 0;

		rarity = itemData->rarity;

		texturePath = itemData->texturePath;
	}
	virtual ~Item() {}

	virtual void PickUp(Player* player) = 0;

	std::string name;
	std::string description;

	int price;
	ItemRarity rarity;

	std::string texturePath;

	// Create a new item, jordi not gonn like this... too bad
	// THIS ALLOCATES MEMORY THAT NEEDS TO BE FREED, DONT FORGET
	static Item* CreateItem(const ItemData* const itemData, bool toBuy = true);

	// Find an Item in the json file given its id
	static const ItemData* const FindItem(const std::vector<ItemData*> items, const int num)
	{
		for (uint i = 0; i < items.size(); ++i)
			if (num >= items[i]->min && num <= items[i]->max)
				return items[i];
		return nullptr;
	}
	// Find an Item in the json file given its name and rarity
	static const ItemData* const FindItem(const std::vector<ItemData*> items, const std::string name, ItemRarity rarity)
	{
		for (uint i = 0; i < items.size(); ++i)
			if (items[i]->name == name && items[i]->rarity == rarity)
				return items[i];
		return nullptr;
	}
};

class AmplifierBarrel : public Item
{
public:

	AmplifierBarrel(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		damageIncrease = itemData->power;
	}
	virtual ~AmplifierBarrel() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::DAMAGE_MODIFY, damageIncrease, 0.0f);
	}

	float damageIncrease;
};

class ElectrocutingPulse : public Item
{
public:

	ElectrocutingPulse(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		stunDuration = itemData->duration;
		stunChance = itemData->chance;
	}
	virtual ~ElectrocutingPulse() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::STUN_BULLETS, stunChance, stunDuration);
	}

	float stunChance;
	float stunDuration;
};

class ColdBullets : public Item
{
public:

	ColdBullets(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		speedSlow = itemData->power;
		slowDuration = itemData->duration;
	}
	virtual ~ColdBullets() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::FREEZE_BULLETS, speedSlow, slowDuration);
	}

	float speedSlow;
	float slowDuration;
};

class LongBarrel : public Item
{
public:

	LongBarrel(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		bulletLifeTimeIncrease = itemData->power;
	}
	virtual ~LongBarrel() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::BULLET_LIFETIME_MODIFY, bulletLifeTimeIncrease, 0.0f);
	}

	float bulletLifeTimeIncrease;
};

class RapidFire : public Item
{
public:

	RapidFire(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		fireRateIncrease = itemData->power;
	}
	virtual ~RapidFire() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::FIRERATE_MODIFY, fireRateIncrease, 0.0f);
	}

	float fireRateIncrease;
};

class ExtendedMagazine : public Item
{
public:

	ExtendedMagazine(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		capacityIncrease = itemData->power;
		reloadTimeIncrease = itemData->duration;
	}
	virtual ~ExtendedMagazine() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::MAXAMMO_MODIFY, capacityIncrease, 0.0f);
		weapon->AddPerk(PerkType::RELOAD_TIME_MODIFY, reloadTimeIncrease, 0.0f);
	}

	float capacityIncrease;
	float reloadTimeIncrease;
};

class FastMagazine : public Item
{
public:

	FastMagazine(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		reloadTimeDecrease = itemData->power;
	}
	virtual ~FastMagazine() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::RELOAD_TIME_MODIFY, reloadTimeDecrease, 0.0f);
	}

	float reloadTimeDecrease;
};

class StimPack : public Item
{
public:

	StimPack(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		healAmount = itemData->power;
	}
	virtual ~StimPack() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		player->AddEffect(EffectType::HEAL, healAmount);
	}

	float healAmount;
};

class DurasteelReinforcement : public Item
{
public:

	DurasteelReinforcement(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		maxHeathIncrease = itemData->power;
	}
	virtual ~DurasteelReinforcement() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		player->AddEffect(EffectType::MAX_HEALTH_MODIFY, maxHeathIncrease, true);
	}

	float maxHeathIncrease;
};

class PropulsedBoots : public Item
{
public:

	PropulsedBoots(const ItemData* const itemData, bool toBuy) : Item(itemData, toBuy)
	{
		speedIncrease = itemData->power;
	}
	virtual ~PropulsedBoots() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		player->AddEffect(EffectType::SPEED_MODIFY, speedIncrease, true);
	}

	float speedIncrease;
};