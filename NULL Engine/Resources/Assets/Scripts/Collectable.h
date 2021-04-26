#pragma once
#include "Object.h"

class Player;

class C_AudioSource;

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
	C_AudioSource* buyAudio = nullptr;

protected:

	virtual void Contact(Player* player) = 0;
};