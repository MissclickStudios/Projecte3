#include "JSONParser.h"

#include "Application.h"

#include "C_2DAnimator.h"
#include "R_Texture.h"

#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "FileSystemDefinitions.h"

C_2DAnimator::C_2DAnimator(GameObject* owner) : Component(owner, ComponentType::ANIMATOR2D)
{
	animationTimer.Stop();
	animationCounter = 0;
	animationStepTime = 30;

	animationLoop = false;
	animationPlaying = false;
	playAnimation = false;

	playFromTheStartOnLoop = false;

	name = "";
}

C_2DAnimator::~C_2DAnimator()
{
}

bool C_2DAnimator::Update()
{
	if (App->gameState != GameState::PLAY)
		return true;


	if (playFromTheStartOnLoop) 
	{
		PlayAnimation(true);
		playFromTheStartOnLoop = false;
	}

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
		if (animationStepTime <= animationTimer.Read() && animationCounter < animation.size() - 1)
		{
			animationCounter++;
			animationTimer.Stop();
			animationTimer.Start();
		}
	
	//Set the texture id of the current animation frame
	if (animation.size() > 0)
		currentFrameIdTexture = GetTextureIdFromVector(animationCounter);

	if (animationCounter == animation.size() - 1)
	{
		if (!animationLoop)
		{
			animationPlaying = false;
			animationCounter = 0;
			playAnimation = false;
			animationTimer.Stop();
		}
		else 
		{
			animationPlaying = true;
			animationCounter = 0;
			playAnimation = false;
			animationTimer.Stop();
			animationTimer.Start();
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
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("Animation Step Time", (uint)animationStepTime);
	root.SetBool("Animation Loop", animationLoop);
	root.SetBool("Animation Set On Loop From Start", playFromTheStartOnLoop);

	root.SetString("Name", name.c_str());

	return true;
}

bool C_2DAnimator::LoadState(ParsonNode& root)
{
	animationStepTime = (uint)root.GetNumber("Animation Step Time");
	animationLoop = root.GetBool("Animation Loop");
	playFromTheStartOnLoop = root.GetBool("Animation Set On Loop From Start");

	name = root.GetString("Name");
	GetAnimationSprites(name.c_str());

	return true;
}

uint C_2DAnimator::GetIdFromAnimation()
{
	return currentFrameIdTexture;
}

void C_2DAnimator::SetAnimationStepTime(int time)
{
	animationStepTime = time;
}

int C_2DAnimator::GetAnimationStepTime()
{
	return animationStepTime;
}

bool C_2DAnimator::IsAnimationPlaying()
{
	return animationPlaying;
}

void C_2DAnimator::SetAnimationPlayFromStart(bool x)
{
	playFromTheStartOnLoop = x;
}

bool C_2DAnimator::GetAnimationPlayFromStart()
{
	return playFromTheStartOnLoop;
}

const char* C_2DAnimator::GetName()
{
	return name.c_str();
}

void C_2DAnimator::ChangeName(const char* name)
{
	this->name = name;
}

void C_2DAnimator::GetAnimationSprites(const char* name)
{
	animation.clear();
	ChangeName(name);
	App->resourceManager->GetAllTextures(animation, name);
}

void C_2DAnimator::PlayAnimation(bool loop)
{
	playAnimation = true;
	animationLoop = loop;
}

void C_2DAnimator::StopAnimation()
{
}

void C_2DAnimator::AddTexture(R_Texture* tex)
{
	animation.push_back(tex);
}

uint C_2DAnimator::GetTextureIdFromVector(int index)
{
	return animation[index]->GetTextureID();
}



