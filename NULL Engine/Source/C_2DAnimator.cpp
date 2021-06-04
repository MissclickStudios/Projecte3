#include "JSONParser.h"
#include "Profiler.h"
#include "Application.h"

#include "C_2DAnimator.h"
#include "R_Texture.h"

#include "Spritesheet.h"

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

	spritesheet = nullptr;
	spritesheet2 = nullptr;
	spritesheet3 = nullptr;

	//ANIMATION 2D TESTING
	spritesheet = new Spritesheet((R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/ChangeWeapon.png"));
	spritesheet->rows = 4;
	spritesheet->columns = 3;
	spritesheet->animationNumber = 12;

	spritesheet2 = new Spritesheet((R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/ChangeWeapon.png"));
	spritesheet2->rows = 5;
	spritesheet2->columns = 5;
	spritesheet2->animationNumber = 25;

	spritesheet3 = new Spritesheet((R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/ChangeWeapon.png"));
	spritesheet3->rows = 5;
	spritesheet3->columns = 5;
	spritesheet3->animationNumber = 25;


}

C_2DAnimator::~C_2DAnimator()
{
	if (spritesheet)
		delete spritesheet;
	if (spritesheet2)
		delete spritesheet2;
	if (spritesheet3)
		delete spritesheet3;
}

bool C_2DAnimator::Update()
{
	OPTICK_CATEGORY("C_2DAnimator Update", Optick::Category::Update);

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

	root.SetNumber("Spritesheet Rows", (uint)spritesheet->rows);
	root.SetNumber("Spritesheet Columns", (uint)spritesheet->columns);
	root.SetNumber("Spritesheet Animation Number", (uint)spritesheet->animationNumber);

	root.SetNumber("Spritesheet2 Rows", (uint)spritesheet2->rows);
	root.SetNumber("Spritesheet2 Columns", (uint)spritesheet2->columns);
	root.SetNumber("Spritesheet2 Animation Number", (uint)spritesheet2->animationNumber);

	root.SetNumber("Spritesheet3 Rows", (uint)spritesheet3->rows);
	root.SetNumber("Spritesheet3 Columns", (uint)spritesheet3->columns);
	root.SetNumber("Spritesheet3 Animation Number", (uint)spritesheet3->animationNumber);

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

	
	spritesheet->rows = (uint)root.GetNumber("Spritesheet Rows");
	spritesheet->columns = (uint)root.GetNumber("Spritesheet Columns");
	spritesheet->animationNumber = (uint)root.GetNumber("Spritesheet Animation Number");

	spritesheet2->rows = (uint)root.GetNumber("Spritesheet2 Rows");
	spritesheet2->columns = (uint)root.GetNumber("Spritesheet2 Columns");
	spritesheet2->animationNumber = (uint)root.GetNumber("Spritesheet2 Animation Number");
			   
	spritesheet3->rows = (uint)root.GetNumber("Spritesheet3 Rows");
	spritesheet3->columns = (uint)root.GetNumber("Spritesheet3 Columns");
	spritesheet3->animationNumber = (uint)root.GetNumber("Spritesheet3 Animation Number");

	return true;
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

/*void C_2DAnimator::SetSpritesheetTexture(R_Texture* spritesheet, int animationNumber)
{
	switch (animationNumber) 
	{
	case 1:
		if (this->spritesheet)
			delete this->spritesheet;
		this->spritesheet = new Spritesheet(spritesheet);
		break;
	case 2:
		if (this->spritesheet2)
			delete this->spritesheet2;
		this->spritesheet2 = new Spritesheet(spritesheet);
		break;
	case 3:
		if (this->spritesheet3)
			delete this->spritesheet3;
		this->spritesheet3 = new Spritesheet(spritesheet);
		break;
	case 0:
		break;
	}
}*/

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
	/*switch (animationDestination) 
	{
	case 1:	
		animation.clear();
		ChangeName(name,1);
		App->resourceManager->GetAllTextures(animation, name);
		if (!animation.empty() && spritesheet != nullptr)
		spritesheet->spriteSheet = animation[0];
		break;
	case 2:
		animation1.clear();
		ChangeName(name,2);
		App->resourceManager->GetAllTextures(animation1, name);
		if (!animation1.empty() && spritesheet2 != nullptr)
		spritesheet2->spriteSheet = animation1[0];
		break;
	case 3:
		animation2.clear();
		ChangeName(name, 3);
		App->resourceManager->GetAllTextures(animation2, name);
		if (!animation2.empty() && spritesheet3 != nullptr)
		spritesheet3->spriteSheet = animation2[0];
		break;
	case 0:
		break;
	}*/

	std::vector<R_Texture*>texturesWithName;
	switch (animationDestination)
	{
	case 1:
		ChangeName(name, 1);
		if (strcmp(name, ""))
			App->resourceManager->GetAllTextures(texturesWithName, name);
		if (!texturesWithName.empty()) 
		{
			App->resourceManager->FreeResource(spritesheet->spriteSheet->GetUID());
			spritesheet->spriteSheet = texturesWithName[0];
		}
		break;
	case 2:
		ChangeName(name, 2);
		if (strcmp(name, ""))
			App->resourceManager->GetAllTextures(texturesWithName, name);
		if (!texturesWithName.empty()) 
		{
			App->resourceManager->FreeResource(spritesheet2->spriteSheet->GetUID());
			spritesheet2->spriteSheet = texturesWithName[0];
		}
		break;
	case 3:
		ChangeName(name, 3);
		if (strcmp(name, ""))
			App->resourceManager->GetAllTextures(texturesWithName, name);
		if (!texturesWithName.empty()) 
		{
			App->resourceManager->FreeResource(spritesheet3->spriteSheet->GetUID());
			spritesheet3->spriteSheet = texturesWithName[0];
		}
		break;
	case 0:
		break;
	}
}

void C_2DAnimator::PlayAnimation(bool loop, int animationNumber)
{
	if (!this) return;
	animationNumberPlaying = animationNumber;
	playAnimation = true;
	animationLoop = loop;
}

int C_2DAnimator::GetAnimationNumber()
{
	return animationNumberPlaying;
}

void C_2DAnimator::LoopAnimation(int animationNum)
{
	switch (animationNum)
	{
	case 1:
		if (spritesheet != nullptr)
		{
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
			if (animationStepTime <= animationTimer.Read() && animationCounter < spritesheet->animationNumber - 1)
			{
				animationCounter++;
				animationTimer.Stop();
				animationTimer.Start();
			}

		//Set the texture coordinates of the current frame
		if (spritesheet->animationNumber > 0 && animationPlaying)
			spritesheet->SetCurrentFrameLocation(animationCounter + 1);

		if (animationCounter == spritesheet->animationNumber - 1)
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
	}
		break;

	case 2:
		if (spritesheet2 != nullptr)
		{
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
			if (animationStepTime <= animationTimer.Read() && animationCounter < spritesheet2->animationNumber - 1)
			{
				animationCounter++;
				animationTimer.Stop();
				animationTimer.Start();
			}

		//Set the texture coordinates of the current frame
		if (spritesheet2->animationNumber > 0 && animationPlaying)
			spritesheet2->SetCurrentFrameLocation(animationCounter + 1);

		if (animationCounter == spritesheet2->animationNumber - 1)
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
	}
		break;

	case 3:
		if (spritesheet3 != nullptr)
		{
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
				if (animationStepTime <= animationTimer.Read() && animationCounter < spritesheet3->animationNumber - 1)
				{
					animationCounter++;
					animationTimer.Stop();
					animationTimer.Start();
				}

			//Set the texture coordinates of the current frame
			if (spritesheet3->animationNumber > 0 && animationPlaying)
				spritesheet3->SetCurrentFrameLocation(animationCounter + 1);

			if (animationCounter == spritesheet3->animationNumber - 1)
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
		}
		break;

	case 0:

		break;
	}
	
}

