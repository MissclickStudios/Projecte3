#include "Player.h"
#include "Weapon.h"

#include "JSONParser.h"

#include <string>

#define EFFECTS_STATS_ARRAY_NAME "Effects Stats"

enum class ItemRarity
{
	COMMON,
	RARE,
	EPIC,
	UNIQUE
};

class Item
{
public:

	Item(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : name(name), rarity(rarity)
	{
		name = itemNode.GetString("Name");
		description = itemNode.GetString("Description");
		if (toBuy)
			price = itemNode.GetArray("Price").GetNumber((uint)rarity);
		else
			price = 0;
	}
	virtual ~Item() {}

	virtual void PickUp(Player* player) = 0;

	std::string name;
	std::string description;

	int price;
	ItemRarity rarity;

	// Create a new item, jordi not gonn like this... too bad
	// THIS ALLOCATES MEMORY THAT NEEDS TO BE FREED, DONT FORGET
	static Item* CreateItem(ParsonNode itemNode, ItemRarity rarity, bool toBuy)
	{
		std::string name = itemNode.GetString("Name");
		if (name == "Amplifier Barrel")
			return (Item*)new AmplifierBarrel(itemNode, rarity, toBuy);
		else if (name == "Electrocuting Pulse")
			return (Item*)new ElectrocutingPulse(itemNode, rarity, toBuy);
		else if (name == "Cold Bullets")
			return (Item*)new ColdBullets(itemNode, rarity, toBuy);
		else if (name == "Long Barrel")
			return (Item*)new LongBarrel(itemNode, rarity, toBuy);
		else if (name == "Rapid Fire")
			return (Item*)new RapidFire(itemNode, rarity, toBuy);
		else if (name == "Extended Magazine")
			return (Item*)new ExtendedMagazine(itemNode, rarity, toBuy);
		else if (name == "Fast Magazine")
			return (Item*)new FastMagazine(itemNode, rarity, toBuy);
		else if (name == "Stim Pack")
			return (Item*)new StimPack(itemNode, rarity, toBuy);
		else if (name == "Durasteel Reinforcement")
			return (Item*)new DurasteelReinforcement(itemNode, rarity, toBuy);
		else if (name == "Propulsed Boots")
			return (Item*)new PropulsedBoots(itemNode, rarity, toBuy);
		
	}

	// Find an Item in the json file given its name
	static ParsonNode FindNode(ParsonArray itemArray, std::string name)
	{
		ParsonNode itemNode;
		for (uint i = 0; i < itemArray.size; ++i)
		{
			itemNode = itemArray.GetNode(i);
			if (itemNode.GetString("Name") == name)
				return itemNode;
		}
		return ParsonNode();
	}
};

class AmplifierBarrel : public Item
{
public:

	AmplifierBarrel(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		damageIncrease = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNumber((uint)rarity);
	}
	virtual ~AmplifierBarrel() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::DAMAGE_MODIFY, new float(damageIncrease));
	}

	float damageIncrease;
};

class ElectrocutingPulse : public Item
{
public:

	ElectrocutingPulse(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		ParsonNode effectDataNode = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNode((uint)rarity);
		stunChance = effectDataNode.GetNumber("Stun Chance");
		stunDuration = effectDataNode.GetNumber("Stun Duration");
	}
	virtual ~ElectrocutingPulse() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::STUN_BULLETS, new std::pair<float, float>(stunChance, stunDuration));
	}

	float stunChance;
	float stunDuration;
};

class ColdBullets : public Item
{
public:

	ColdBullets(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		ParsonNode effectDataNode = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNode((uint)rarity);
		speedSlow = effectDataNode.GetNumber("Speed Slow");
		slowDuration = effectDataNode.GetNumber("Slow Duration");
	}
	virtual ~ColdBullets() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::FREEZE_BULLETS, new std::pair<float, float>(speedSlow, slowDuration));
	}

	float speedSlow;
	float slowDuration;
};

class LongBarrel : public Item
{
public:

	LongBarrel(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		bulletLifeTimeIncrease = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNumber((uint)rarity);
	}
	virtual ~LongBarrel() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::BULLET_LIFETIME_MODIFY, new float(bulletLifeTimeIncrease));
	}

	float bulletLifeTimeIncrease;
};

class RapidFire : public Item
{
public:

	RapidFire(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		fireRateIncrease = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNumber((uint)rarity);
	}
	virtual ~RapidFire() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::FIRERATE_MODIFY, new float(fireRateIncrease));
	}

	float fireRateIncrease;
};

class ExtendedMagazine : public Item
{
public:

	ExtendedMagazine(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		ParsonNode effectDataNode = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNode((uint)rarity);
		capacityIncrease = effectDataNode.GetNumber("Capacity Increase");
		reloadTimeIncrease = effectDataNode.GetNumber("Reload Time Decrease");
	}
	virtual ~ExtendedMagazine() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::MAXAMMO_MODIFY, new float(capacityIncrease));
		weapon->AddPerk(PerkType::RELOAD_TIME_MODIFY, new float(reloadTimeIncrease));
	}

	float capacityIncrease;
	float reloadTimeIncrease;
};

class FastMagazine : public Item
{
public:

	FastMagazine(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		reloadTimeDecrease = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNumber((uint)rarity);
	}
	virtual ~FastMagazine() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon == nullptr)
			return;

		weapon->AddPerk(PerkType::RELOAD_TIME_MODIFY, new float(reloadTimeDecrease));
	}

	float reloadTimeDecrease;
};

class StimPack : public Item
{
public:

	StimPack(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		healAmount = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNumber((uint)rarity);
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

	DurasteelReinforcement(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		maxHeathIncrease = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNumber((uint)rarity);
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

	PropulsedBoots(ParsonNode itemNode, ItemRarity rarity, bool toBuy) : Item(itemNode, rarity, toBuy)
	{
		speedIncrease = itemNode.GetArray(EFFECTS_STATS_ARRAY_NAME).GetNumber((uint)rarity);
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