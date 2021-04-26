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

	void OnCollisionEnter(GameObject* object) override;

	int price = 0;

	bool used = false;

	// Audio 
	std::string buyAudio;

protected:

	virtual void Contact(Player* player) = 0;
};