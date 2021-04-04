#include "C_2DAnimator.h"

C_2DAnimator::C_2DAnimator(GameObject* owner) : Component(owner, ComponentType::ANIMATOR2D)
{
}

C_2DAnimator::~C_2DAnimator()
{
}

bool C_2DAnimator::Update()
{
	for (auto animationd = animation.cbegin(); animationd != animation.cend(); ++animationd)
	{
	}
	return true;
}

bool C_2DAnimator::CleanUp()
{
	return false;
}

bool C_2DAnimator::SaveState(ParsonNode& root) const
{
	return false;
}

bool C_2DAnimator::LoadState(ParsonNode& root)
{
	return false;
}
