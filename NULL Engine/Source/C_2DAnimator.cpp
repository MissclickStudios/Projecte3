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

	animationNumberPlaying = 1;

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
		PlayAnimation(true,1);
		playFromTheStartOnLoop = false;
	}

	LoopAnimation(animationNumberPlaying);

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
	root.SetString("Name1", name1.c_str());
	root.SetString("Name2", name2.c_str());

	return true;
}

bool C_2DAnimator::LoadState(ParsonNode& root)
{
	animationStepTime = (uint)root.GetNumber("Animation Step Time");
	animationLoop = root.GetBool("Animation Loop");
	playFromTheStartOnLoop = root.GetBool("Animation Set On Loop From Start");

	name = root.GetString("Name");
	name1 = root.GetString("Name1");
	name2 = root.GetString("Name2");

	GetAnimationSprites(name.c_str(),1);
	GetAnimationSprites(name1.c_str(), 2);
	GetAnimationSprites(name2.c_str(), 3);

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

const char* C_2DAnimator::GetName(int animationNum)
{
	switch (animationNum)
	{
	case 1:
		return name.c_str();
		break;
	case 2:
		return name1.c_str();
		break;
	case 3:
		return name2.c_str();
		break;
	case 0:
		break;
	}
}

void C_2DAnimator::ChangeName(const char* name, int animationNum)
{
	switch (animationNum)
	{
	case 1:
		this->name = name;
		break;
	case 2:
		this->name1 = name;
		break;
	case 3:
		this->name2 = name;
		break;
	case 0:
		break;
	}
}

void C_2DAnimator::GetAnimationSprites(const char* name, int animationDestination)
{
	switch (animationDestination) 
	{
	case 1:	
		animation.clear();
		ChangeName(name,1);
		App->resourceManager->GetAllTextures(animation, name);
		break;
	case 2:
		animation1.clear();
		ChangeName(name,2);
		App->resourceManager->GetAllTextures(animation1, name);
		break;
	case 3:
		animation2.clear();
		ChangeName(name, 3);
		App->resourceManager->GetAllTextures(animation2, name);
		break;
	case 0:
		break;
	}

}

void C_2DAnimator::PlayAnimation(bool loop, int animationNumber)
{
	animationNumberPlaying = animationNumber;
	playAnimation = true;
	animationLoop = loop;
}

void C_2DAnimator::StopAnimation()
{
}

uint C_2DAnimator::GetTextureIdFromVector(int index, int animationNum)
{
	switch (animationNum) {
	case 1:
		return animation[index]->GetTextureID();
		break;
	case 2:
		return animation1[index]->GetTextureID();
		break;
	case 3:
		return animation2[index]->GetTextureID();
		break;
	case 0:
		return animation[index]->GetTextureID();
		break;
	}

}

void C_2DAnimator::LoopAnimation(int animationNum)
{
	switch (animationNum) 
	{
	case 1:
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
			currentFrameIdTexture = GetTextureIdFromVector(animationCounter,1);

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
		break;
	case 2:
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
			if (animationStepTime <= animationTimer.Read() && animationCounter < animation1.size() - 1)
			{
				animationCounter++;
				animationTimer.Stop();
				animationTimer.Start();
			}

		//Set the texture id of the current animation frame
		if (animation1.size() > 0)
			currentFrameIdTexture = GetTextureIdFromVector(animationCounter,2);

		if (animationCounter == animation1.size() - 1)
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
		break;
	case 3:
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
			if (animationStepTime <= animationTimer.Read() && animationCounter < animation2.size() - 1)
			{
				animationCounter++;
				animationTimer.Stop();
				animationTimer.Start();
			}

		//Set the texture id of the current animation frame
		if (animation2.size() > 0)
			currentFrameIdTexture = GetTextureIdFromVector(animationCounter, 3);

		if (animationCounter == animation2.size() - 1)
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
		break;
	case 0:
		break;
	}
	
}

Spritesheet::Spritesheet(R_Texture* spritesheet)
{
	spriteSheet = spritesheet;
	columns = 0;
	rows = 0;
	pixelHeight = 0;
	pixelLenght = 0;
}

Spritesheet::~Spritesheet()
{
}

void Spritesheet::SetSpritesheetSize(int s_rows, int s_columns, int s_pisxelHeight, int s_pixelLenght)
{
	columns = s_columns;
	rows = s_rows;
	pixelHeight = s_pisxelHeight;
	pixelLenght = s_pixelLenght;
}

void Spritesheet::GetFrameProportions(int row, int column)
{
	currentFrame.proportionBeginX = (pixelLenght * row) / pixelLenght * (rows - 1);
	currentFrame.proportionFinalX = (pixelLenght * (row + 1)) / pixelLenght * (rows - 1);

	currentFrame.proportionBeginY = (pixelHeight * column) / pixelHeight * (columns - 1);
	currentFrame.proportionBeginY = (pixelHeight * column + 1) / pixelHeight * (columns - 1);
}

void Spritesheet::SetCurrentFrameLocation(int frameNumber)
{
	int rowN = 0;
	int columnN = 0;

	for (int i = 0; i <= rows; i++)
	{
		for (int k = 0; k <= rows; k++)
		{
			if(frameNumber == 0)
			{
				GetFrameProportions(rowN, columnN);
			}


			frameNumber--;
			columnN++;
		}
		columnN = 0;
		rowN++;
	}
}
