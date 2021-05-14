#pragma once
#include "Object.h"

class Item;

class ShopItem : public Object
{
public:

	ShopItem();
	virtual ~ShopItem();

	void Start() override;
	void Update() override;
	void CleanUp() override;

	void OnPause() override;
	void OnResume() override;

	void OnTriggerRepeat(GameObject* object) override;

	Item* item = nullptr;

protected:

	bool used = false;
};