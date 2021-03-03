#include "C_PlayerController.h"
#include "Component.h"

C_PlayerManager::C_PlayerManager(GameObject* owner) : Component(owner, ComponentType::PLAYER_CONTROLLER)
{
}

C_PlayerManager::~C_PlayerManager()
{
}

bool C_PlayerManager::Update()
{
	return false;
}

bool C_PlayerManager::CleanUp()
{
	return false;
}

void C_PlayerManager::Move()
{
}

void C_PlayerManager::Rotate()
{
}

void C_PlayerManager::OnCollision()
{
}
