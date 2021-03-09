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

	void		GetRooms();
	void		GenerateLevel();
	void		GenerateRoom(int room);
	void		HandleRoomGeneration();
	void		CleanUp();

public:
	std::vector<std::string>			allRooms;
	std::vector<std::string>			levelRooms;
	float		roomsToAdd;
	int			roomNum;

};
#endif //__LEVELGENERATOR_H__