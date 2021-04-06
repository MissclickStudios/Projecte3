#include "Application.h"

#include "C_2DAnimator.h"
#include "R_Texture.h"

C_2DAnimator::C_2DAnimator(GameObject* owner) : Component(owner, ComponentType::ANIMATOR2D)
{
	animationTimer.Stop();
	animationCounter = 0;
	animationFrames = 0;
	animationStepTime = 0;

	animationLoop = false;
	animationPlaying = false;
	playAnimation = false;
}

C_2DAnimator::~C_2DAnimator()
{
}

bool C_2DAnimator::Update()
{
	if (App->gameState != GameState::PLAY)
		return true;

	//Start a new animation
	if (playAnimation) 
	{
		animationPlaying = true;
		animationCounter = 0;
		animationTimer.Start();
		playAnimation = false;
	}

	//Animation loop
	if (animationPlaying) 
		if (animationStepTime <= animationTimer.Read() && animationCounter <= animationFrames)
			animationCounter++;

	//Set the texture id of the current animation frame
	currentFrameIdTexture = GetTextureIdFromVector(animationCounter);

	if (animationCounter == animationFrames)
	{
		if (!animationLoop)
		{
			animationPlaying = false;
			animationCounter = 0;
			playAnimation = false;
		}
		else 
		{
			animationPlaying = true;
			animationCounter = 0;
			playAnimation = false;
		}
	}

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
	return currentFrameIdTexture;
}

void C_2DAnimator::PlayAnimation(bool loop)
{
	playAnimation = true;
	animationLoop = loop;
}

uint C_2DAnimator::GetTextureIdFromVector(int index)
{
	return animation[index]->GetTextureID();
}



