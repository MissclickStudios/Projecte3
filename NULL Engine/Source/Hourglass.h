#ifndef __HOURGLASS_H__
#define __HOURGLASS_H__

#include <string>

typedef unsigned char		uchar;
typedef unsigned int		uint;

struct Hourglass																	// This struct is called Hourglass instead of Clock due to Linker conflicts with MathGeoLib.
{
	//Hourglass();
	Hourglass(uint hours = 0, uchar minutes = 0, float seconds = 0.0f);

	//void Update();
	void Update(uint ms);
	void ResetClock();

	std::string GetTimeAsString();

	uint	hours;
	uchar	minutes;
	float	seconds;

	uint previous_ticks;
};

#endif // !__HOURGLASS_H__