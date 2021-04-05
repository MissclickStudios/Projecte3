#ifndef __PREFAB_H__
#define __PREFAB_H__

#include <string>
typedef unsigned int uint;

struct Prefab
{
	Prefab();
	Prefab(uint uid, const char* name,int updateTime);
	uint uid = 0;
	std::string name;
	int updateTime = 0; //In Unix

};

#endif // !__PREFAB_H__