#ifndef __LEVELGENERATOR_H__
#define __LEVELGENERATOR_H__

#include "Globals.h"
#include <vector>
#include <string>
class R_Scene;

class LevelGenerator
{
public:

	LevelGenerator();
	~LevelGenerator();

	//Get all the scenes saved in the assets folder with the string "Room" in the name
	void		GetRooms();

	//Builds the level randomly with all the scenes save with the string "Room" in the name
	void		GenerateLevel();
	
	//Spawn the room designated by the integer passed
	void		GoNextRoom();

	void		GoPreviousRoom();

	void		InitiateLevel(int level);

	//Add an special room with a name and position, if the position exceeds the level size it places on last position
	void		AddFixedRoom(std::string name, int level,int position); 

	//Handle how and when to load the following or previous room/scene
	void		HandleRoomGeneration();

	void		CleanUp();

public:
	std::vector<std::string>			allRooms;
	std::vector<std::string>			level1;
	std::vector<std::string>			level2;
	int			currentLevel;
	float		roomsToAdd;
	int			roomNum;

};
#endif //__LEVELGENERATOR_H__