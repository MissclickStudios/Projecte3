#include "Player.h"
#include "Weapon.h"

#include "ItemRarity.h"

#include "JSONParser.h"

#include <string>

#define EFFECTS_STATS_ARRAY_NAME "Effects Stats"

// Data that acuratelly stores all information of an item stored in an item pool
struct ItemData
{
	ItemData() : name(""), description(""), price(0), rarity(ItemRarity::COMMON), power(0.0f), duration(0.0f), chance(0.0f), minimum(0), maximum(0), texturePath("") {}
	ItemData(std::string name, std::string description, int price, ItemRarity rarity, float power, float duration, float chance, int minimum, int maximum, std::string texturePath)
		: name(name), description(description), price(price), rarity(rarity), power(power), duration(duration), chance(chance), minimum(minimum), maximum(maximum), texturePath(texturePath) {}

	std::string name;
	std::string description;
	int price;
	ItemRarity rarity;

	float power;
	float duration;
	float chance;

	int minimum;
	int maximum;

	std::string texturePath;
};

class Item
{
public:

	Item(ItemData* itemData, bool toBuy) : data(itemData)
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

	ItemData* data = nullptr;

	bool toSave = true;

	// Create a new item, jordi not gonn like this... too bad
	// THIS ALLOCATES MEMORY THAT NEEDS TO BE FREED, DONT FORGET
	static Item* CreateItem(ItemData* itemData, bool toBuy = true);

	// Find an Item in the json file given its id
	static ItemData* FindItem(const std::vector<ItemData*> items, const int num)
	{
		for (uint i = 0; i < items.size(); ++i)
			if (num >= items[i]->minimum && num <= items[i]->maximum)
				return items[i];
		return nullptr;
	}
	// Find an Item in the json file given its name and rarity
	static ItemData* FindItem(const std::vector<ItemData*> items, const std::string name, ItemRarity rarity)
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

	AmplifierBarrel(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		damageIncrease = itemData->power;
	}
	virtual ~AmplifierBarrel() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetPrimaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::DAMAGE_MODIFY, damageIncrease, 0.0f);

		weapon = player->GetSecondaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::DAMAGE_MODIFY, damageIncrease, 0.0f);
	}

	float damageIncrease;
};

class ElectrocutingPulse : public Item
{
public:

	ElectrocutingPulse(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		stunDuration = itemData->duration;
		stunChance = itemData->chance;
	}
	virtual ~ElectrocutingPulse() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetPrimaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::STUN_BULLETS, stunChance, stunDuration);

		weapon = player->GetSecondaryWeapon();
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

	ColdBullets(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		speedSlow = itemData->power;
		slowDuration = itemData->duration;
	}
	virtual ~ColdBullets() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetPrimaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::FREEZE_BULLETS, speedSlow, slowDuration);

		weapon = player->GetSecondaryWeapon();
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

	LongBarrel(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		bulletLifeTimeIncrease = itemData->power;
	}
	virtual ~LongBarrel() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetPrimaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::BULLET_LIFETIME_MODIFY, bulletLifeTimeIncrease, 0.0f);

		weapon = player->GetSecondaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::BULLET_LIFETIME_MODIFY, bulletLifeTimeIncrease, 0.0f);
	}

	float bulletLifeTimeIncrease;
};

class RapidFire : public Item
{
public:

	RapidFire(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		fireRateIncrease = itemData->power;
	}
	virtual ~RapidFire() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetPrimaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::FIRERATE_MODIFY, fireRateIncrease, 0.0f);

		weapon = player->GetSecondaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::FIRERATE_MODIFY, fireRateIncrease, 0.0f);
	}

	float fireRateIncrease;
};

class ExtendedMagazine : public Item
{
public:

	ExtendedMagazine(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		capacityIncrease = itemData->power;
		reloadTimeIncrease = itemData->duration;
	}
	virtual ~ExtendedMagazine() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetPrimaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::MAXAMMO_MODIFY, capacityIncrease, 0.0f);
		weapon->AddPerk(PerkType::RELOAD_TIME_MODIFY, reloadTimeIncrease, 0.0f);

		weapon = player->GetSecondaryWeapon();
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

	FastMagazine(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		reloadTimeDecrease = itemData->power;
	}
	virtual ~FastMagazine() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetPrimaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::RELOAD_TIME_MODIFY, reloadTimeDecrease, 0.0f);

		weapon = player->GetSecondaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::RELOAD_TIME_MODIFY, reloadTimeDecrease, 0.0f);
	}

	float reloadTimeDecrease;
};

class StimPack : public Item
{
public:

	StimPack(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		toSave = false;
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

	DurasteelReinforcement(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
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

	PropulsedBoots(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		speedIncrease = itemData->power;
	}
	virtual ~PropulsedBoots() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		player->AddEffect(EffectType::SPEED_MODIFY, 0.0f, true, speedIncrease);
	}

	float speedIncrease;
};

class PremiumTicket : public Item
{
public:

	PremiumTicket(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		priceReduction = itemData->power;
	}
	virtual ~PremiumTicket() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		player->AddEffect(EffectType::PRICE_MODIFY, 0.0f, true, priceReduction);
	}

	float priceReduction;
};

class RefrigerationLiquid : public Item
{
public:

	RefrigerationLiquid(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		cooldownReduction = itemData->power;
	}
	virtual ~RefrigerationLiquid() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		player->AddEffect(EffectType::COOLDOWN_MODIFY, 0.0f, true, cooldownReduction);
	}

	float cooldownReduction;
};

class BeskarIngots : public Item
{
public:
	BeskarIngots(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		toSave = false;
		ingots = (int)itemData->power;
	}
	virtual ~BeskarIngots() {}


	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		player->beskar += ingots;
	}

	int ingots;
};

class GalacticCredit : public Item
{
public:

	GalacticCredit(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		toSave = false;
		credits = (int)itemData->power;
	}
	virtual ~GalacticCredit() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		player->GiveCredits(credits);
	}

	int credits;
};

class JacketBullets : public Item
{
public:

	JacketBullets(ItemData* itemData, bool toBuy) : Item(itemData, toBuy)
	{
		damageMultiplier = itemData->power;
	}
	virtual ~JacketBullets() {}

	void PickUp(Player* player)
	{
		if (player == nullptr)
			return;

		Weapon* weapon = player->GetPrimaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::JACKET_BULLETS, damageMultiplier, 0.0f);

		weapon = player->GetSecondaryWeapon();
		if (weapon == nullptr)
			return;
		weapon->AddPerk(PerkType::JACKET_BULLETS, damageMultiplier, 0.0f);
	}

	float damageMultiplier;
};
