#include "C_2DAnimator.h"
#include "R_Texture.h"

C_2DAnimator::C_2DAnimator(GameObject* owner) : Component(owner, ComponentType::ANIMATOR2D)
{
}

C_2DAnimator::~C_2DAnimator()
{
}

bool C_2DAnimator::Update()
{

	return true;
}

bool C_2DAnimator::CleanUp()
{
	return true;
}

bool C_2DAnimator::SaveState(ParsonNode& root) const
{
	return true;
}

bool C_2DAnimator::LoadState(ParsonNode& root)
{
	return true;
}

uint C_2DAnimator::GetIdFromAnimation()
{
	return uint();
}

uint C_2DAnimator::GetTextureIdFromVector(int index)
{
	return animation[index]->GetTextureID();
}



