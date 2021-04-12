#include "Application.h"
#include "LevelGenerator.h"
#include "Random.h"

#include "M_ResourceManager.h"
#include "M_FileSystem.h"
#include "M_Input.h"
#include "M_Scene.h"

LevelGenerator::LevelGenerator():
roomsToAdd (0),
roomNum	(0),
currentLevel(0)
{

}

LevelGenerator::~LevelGenerator()
{
	allRooms.clear();
	level1.clear();
	level2.clear();

}


void LevelGenerator::GetRooms()
{
	App->fileSystem->GetAllFilesWithFilter(ASSETS_SCENES_PATH, allRooms, "Room", "json");
	roomsToAdd = allRooms.size();
}

void LevelGenerator::GenerateLevel()
{
	float randomNum;
	if (!allRooms.empty())
	{
		while (roomsToAdd > 0)
		{
			randomNum = Random::LCG::GetBoundedRandomUint(0,allRooms.size() - 1);

			if (allRooms[randomNum].find("Level1") != std::string::npos)
			{
				level1.push_back(allRooms[randomNum]);
				allRooms[randomNum].erase();
				roomsToAdd--;
			}
			else if (allRooms[randomNum].find("Level2") != std::string::npos)
			{
				level2.push_back(allRooms[randomNum]);
				allRooms[randomNum].erase();
				roomsToAdd--;
			}
		}	
	}
	else
	{
		LOG("No rooms found to generate the level.");
	}

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
				App->scene->LoadScene(level1[roomNum].c_str());
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
			if (roomNum < level2.size() - 1)
			{
				roomNum++;
				App->scene->LoadScene(level2[roomNum].c_str());
			}
			else
			{
				LOG("End of the level reached.");
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
				App->scene->LoadScene(level1[roomNum].c_str());
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
				App->scene->LoadScene(level2[roomNum].c_str());
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
			App->scene->LoadScene(level1[0].c_str());
			roomNum = 0;
			currentLevel = 1;
		}
	}
	else if (level == 2)
	{
		if (!level2.empty())
		{
			App->scene->LoadScene(level2[0].c_str());
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
		
		if(level == 1)
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
				else if (i > newPosition&& i < level1.size())
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
				else if (i > newPosition&& i < level2.size())
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
			if(currentLevel == 1)
			{
				(roomNum < level1.size() - 1) ? GoNextRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");
			}
			else if (currentLevel == 2)
			{
				(roomNum < level2.size() - 1) ? GoNextRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");
			}
			
		}
		if (App->input->GetKey(SDL_SCANCODE_KP_4) == KeyState::KEY_DOWN)
		{

			if(currentLevel == 1)
			{
				(roomNum < level1.size() - 1) ? GoPreviousRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");
			}
			else if (currentLevel == 2)
			{
				(roomNum < level2.size() - 1) ? GoPreviousRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");
			}
		}
	}
}

void LevelGenerator::CleanUp()
{
}
