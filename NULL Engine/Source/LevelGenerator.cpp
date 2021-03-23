#include "Application.h"
#include "LevelGenerator.h"
#include "Random.h"

#include "M_ResourceManager.h"
#include "M_FileSystem.h"
#include "M_Input.h"
#include "M_Scene.h"

LevelGenerator::LevelGenerator():
roomsToAdd (0),
roomNum	(0)
{
}

LevelGenerator::~LevelGenerator()
{
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
			if (allRooms[randomNum] != "")
			{
				levelRooms.push_back(allRooms[randomNum]);
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

void LevelGenerator::GenerateRoom(int room)
{
	if (!levelRooms.empty())
	{
		App->scene->LoadScene(levelRooms[room].c_str());
	}
}

void LevelGenerator::AddFixedRoom(std::string name, int position)
{
	int newPosition = position - 1;
	std::string roomPath = ASSETS_SCENES_PATH + name + ".json";
	
	
	if (App->fileSystem->Exists(roomPath.c_str()))
	{
		std::string tempRoom;
		std::string tempRoom2;
		levelRooms.resize(levelRooms.size() + 1);

		if (levelRooms.size() <= newPosition)
		{
			newPosition = levelRooms.size();
		}
		for (int i = 0; i <= levelRooms.size(); i++)
		{
			if (i == newPosition)
			{ 
				if (i == levelRooms.size())
				{
					levelRooms[i - 1] = roomPath;
				}
				else 
				{
					tempRoom = levelRooms[i + 1];
					levelRooms[i + 1] = levelRooms[i];
					levelRooms[i] = roomPath;
				}
			}
			else if( i > newPosition && i < levelRooms.size())
			{
				tempRoom2 = levelRooms[i];
				levelRooms[i] = tempRoom;
				if(i + 1 != levelRooms.size()) tempRoom = levelRooms[i + 1];
				tempRoom = tempRoom2;
			}
			
		}
	}
	
}

void LevelGenerator::HandleRoomGeneration()
{
	if (App->input->GetKey(SDL_SCANCODE_KP_6) == KeyState::KEY_DOWN)
	{
		if (roomNum < levelRooms.size()-1)
		{
			roomNum++;
			GenerateRoom(roomNum);
		}
		else
		{
			LOG("End of the level reached.");
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_4) == KeyState::KEY_DOWN)
	{
		if (roomNum > 0)
		{
			roomNum--;
			GenerateRoom(roomNum);
		}
		else
		{
			LOG("Beggining of the level reached.");
		}
	}
}

void LevelGenerator::NextRoom()
{
	if (roomNum < levelRooms.size() - 1)
	{
		roomNum++;
		GenerateRoom(roomNum);
	}
	else
	{
		LOG("End of the level reached.");
	}
}

void LevelGenerator::CleanUp()
{
}
