#include <algorithm>
#include "Application.h"
#include "LevelGenerator.h"
#include "Random.h"

#include "M_ResourceManager.h"
#include "M_FileSystem.h"
#include "M_Input.h"
#include "M_Scene.h"

LevelGenerator::LevelGenerator() :
	roomNum(0),
	currentLevel(0)
{

}

LevelGenerator::~LevelGenerator()
{
	level1.clear();
	level2.clear();
}

void LevelGenerator::GenerateLevel()
{
	std::random_shuffle(level1.begin(), level1.end());
	std::random_shuffle(level2.begin(), level2.end());
}

void LevelGenerator::GoNextRoom()
{
	if (currentLevel == 1)
	{
		if (!level1.empty())
		{
			if (roomNum < level1.size() - 1)
			{
				roomNum++;
				App->scene->ScriptChangeScene(level1[roomNum]);
			}

			else if (roomNum == level1.size() - 1)
			{
				LOG("[SCENE] Level Generator: End of the Game Reached!");

				InitiateLevel(2);
			}
		}
	}
	else if (currentLevel == 2)
	{
		if (!level2.empty())
		{
			if (roomNum < level2.size() - 1)
			{
				roomNum++;
				App->scene->ScriptChangeScene(level2[roomNum]);
			}
			else if (roomNum == level2.size() - 1)
			{
				LOG("[SCENE] Level Generator: End of the Game Reached!");

				InitiateLevel(1);
			}
		}
	}
}

void LevelGenerator::GoPreviousRoom()
{
	if (currentLevel == 1)
	{
		if (!level1.empty())
		{
			if (roomNum > 0)
			{
				roomNum--;
				App->scene->ScriptChangeScene(level1[roomNum]);
			}
			else
			{
				LOG("End of the level reached.");
			}
		}
	}
	else if (currentLevel == 2)
	{
		if (!level2.empty())
		{
			if (roomNum > 0)
			{
				roomNum--;
				App->scene->ScriptChangeScene(level2[roomNum]);
			}
			else
			{
				LOG("End of the level reached.");
			}
		}
	}

}

void LevelGenerator::InitiateLevel(int level)
{
	if (level == 1)
	{
		if (!level1.empty())
		{
			App->scene->ScriptChangeScene(level1[0]);
			roomNum = 0;
			currentLevel = 1;
		}
	}
	else if (level == 2)
	{
		if (!level2.empty())
		{
			App->scene->ScriptChangeScene(level2[0]);
			roomNum = 0;
			currentLevel = 2;
		}
	}
}

void LevelGenerator::AddFixedRoom(std::string name, int level, int position)
{
	int newPosition = position - 1;
	std::string roomPath = ASSETS_SCENES_PATH + name + ".json";

	if (App->fileSystem->Exists(roomPath.c_str()))
	{
		std::string tempRoom;
		std::string tempRoom2;

		if (level == 1)
		{
			level1.resize(level1.size() + 1);

			if (level1.size() <= newPosition)
			{
				newPosition = level1.size();
			}
			for (int i = 0; i <= level1.size(); i++)
			{
				if (i == newPosition)
				{
					if (i == level1.size())
					{
						level1[i - 1] = roomPath;
					}
					else
					{
						tempRoom = level1[i + 1];
						level1[i + 1] = level1[i];
						level1[i] = roomPath;
					}
				}
				else if (i > newPosition && i < level1.size())
				{
					tempRoom2 = level1[i];
					level1[i] = tempRoom;
					if (i + 1 != level1.size()) tempRoom = level1[i + 1];
					tempRoom = tempRoom2;
				}

			}
		}

		else if (level == 2)
		{
			level2.resize(level2.size() + 1);

			if (level2.size() <= newPosition)
			{
				newPosition = level2.size();
			}
			for (int i = 0; i <= level2.size(); i++)
			{
				if (i == newPosition)
				{
					if (i == level2.size())
					{
						level2[i - 1] = roomPath;
					}
					else
					{
						tempRoom = level2[i + 1];
						level2[i + 1] = level2[i];
						level2[i] = roomPath;
					}
				}
				else if (i > newPosition && i < level2.size())
				{
					tempRoom2 = level2[i];
					level2[i] = tempRoom;
					if (i + 1 != level2.size()) tempRoom = level2[i + 1];
					tempRoom = tempRoom2;
				}

			}
		}
	}

}

void LevelGenerator::HandleRoomGeneration()
{
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT)
	{
		if (App->input->GetKey(SDL_SCANCODE_KP_6) == KeyState::KEY_DOWN)
		{

			GoNextRoom();

			/*if(currentLevel == 1)
			{

				(roomNum < level1.size() - 1) ? GoNextRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");

				(roomNum == level1.size()) ? InitiateLevel(2) : LOG("[SCENE] Level Generator: Begin Level 2 ");
			}
			else if (currentLevel == 2)
			{
				(roomNum < level2.size() - 1) ? GoNextRoom() : LOG("[SCENE] Level Generator: End of the Game Reached!");
<<<<<<< Updated upstream
			}*/

		}
		if (App->input->GetKey(SDL_SCANCODE_KP_4) == KeyState::KEY_DOWN)
		{
			if (currentLevel == 1)
			{
				(roomNum > 0) ? GoPreviousRoom() : LOG("[SCENE] Level Generator: Begin of the Level Reached!");

			}
			else if (currentLevel == 2)
			{
				(roomNum > 0) ? GoPreviousRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");

				(roomNum == 0) ? InitiateLevel(1) : LOG("[SCENE] Level Generator: Begin Level 1 ");

			}
		}
	}
}

void LevelGenerator::CleanUp()
{
}