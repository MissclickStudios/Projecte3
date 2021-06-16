#include "Items.h"

Item* Item::CreateItem(ItemData* itemData, bool toBuy)
{
	std::string name = itemData->name;
	if (name == "Amplifier Barrel")
		return (Item*)new AmplifierBarrel(itemData, toBuy);
	else if (name == "Electro Pulse")
		return (Item*)new ElectrocutingPulse(itemData, toBuy);
	else if (name == "Cold Bullets")
		return (Item*)new ColdBullets(itemData, toBuy);
	else if (name == "Long Barrel")
		return (Item*)new LongBarrel(itemData, toBuy);
	else if (name == "Rapid Fire")
		return (Item*)new RapidFire(itemData, toBuy);
	else if (name == "Extended Mag")
		return (Item*)new ExtendedMagazine(itemData, toBuy);
	else if (name == "Fast Magazine")
		return (Item*)new FastMagazine(itemData, toBuy);
	else if (name == "Stim Pack")
		return (Item*)new StimPack(itemData, toBuy);
	else if (name == "Durasteel Reinforcement")
		return (Item*)new DurasteelReinforcement(itemData, toBuy);
	else if (name == "Propulsed Boots")
		return (Item*)new PropulsedBoots(itemData, toBuy);
	else if (name == "Beskar Ingot")
		return (Item*)new BeskarIngots(itemData, toBuy);
	else if (name == "Galactic Credit")
		return (Item*)new GalacticCredit(itemData, toBuy);
	else if (name == "FMJ Bullets")
		return (Item*)new JacketBullets(itemData, toBuy);
	else if (name == "Premium Ticket")
		return (Item*)new PremiumTicket(itemData, toBuy);
	else if (name == "Refrigeration Liquid")
		return (Item*)new RefrigerationLiquid(itemData, toBuy);
	
	return nullptr;
}
