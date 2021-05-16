#include "Items.h"

Item* Item::CreateItem(const ItemData* const itemData, bool toBuy)
{
	std::string name = itemData->name;
	if (name == "Amplifier Barrel")
		return (Item*)new AmplifierBarrel(itemData, toBuy);
	else if (name == "Electrocuting Pulse")
		return (Item*)new ElectrocutingPulse(itemData, toBuy);
	else if (name == "Cold Bullets")
		return (Item*)new ColdBullets(itemData, toBuy);
	else if (name == "Long Barrel")
		return (Item*)new LongBarrel(itemData, toBuy);
	else if (name == "Rapid Fire")
		return (Item*)new RapidFire(itemData, toBuy);
	else if (name == "Extended Magazine")
		return (Item*)new ExtendedMagazine(itemData, toBuy);
	else if (name == "Fast Magazine")
		return (Item*)new FastMagazine(itemData, toBuy);
	else if (name == "Stim Pack")
		return (Item*)new StimPack(itemData, toBuy);
	else if (name == "Durasteel Reinforcement")
		return (Item*)new DurasteelReinforcement(itemData, toBuy);
	else if (name == "Propulsed Boots")
		return (Item*)new PropulsedBoots(itemData, toBuy);

	return nullptr;
}
