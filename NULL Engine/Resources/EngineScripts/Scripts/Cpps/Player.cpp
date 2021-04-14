#include "Player.h"

#include "Application.h"
#include "M_Input.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Start()
{
}

void Player::Update()
{
	if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_DOWN)
		AddEffect(EffectType::FROZEN, 0.5f);
}

void Player::CleanUp()
{
}

void Player::TakeDamage(float damage)
{
}

void Player::Frozen()
{
}
