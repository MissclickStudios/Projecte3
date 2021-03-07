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
	App->fileSystem->GetAllFilesWithExtensionAndName(ASSETS_SCENES_PATH, "json", "Room", allRooms);
	roomsToAdd = allRooms.size();
}

void LevelGenerator::GenerateLevel()
{
	float randomNum;
	if (!allRooms.empty())
	{
		while (roomsToAdd > 0)
		{
			randomNum = Random::LCG::GetBoundedRandomUint(0,3);
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

void LevelGenerator::HandleRoomGeneration()
{
	if (App->input->GetKey(SDL_SCANCODE_N) == KeyState::KEY_DOWN)
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
	if (App->input->GetKey(SDL_SCANCODE_M) == KeyState::KEY_DOWN)
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

void LevelGenerator::CleanUp()
{
}
