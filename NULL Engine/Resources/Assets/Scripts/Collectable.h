#pragma once
#include "Object.h"

class Player;

class Collectable : public Object
{
public:

	Collectable();
	virtual ~Collectable();

	void Start() override;
	void Update() override;
	void CleanUp() override;

	void OnTriggerEnter(GameObject* object) override;

	int price = 0;

	std::string typeName = " ";
	int typeInt = 0;

	bool used = false;

protected:

	virtual void Contact(Player* player) = 0;

	virtual void SetTypeInfo() = 0;
};