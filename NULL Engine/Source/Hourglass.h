#ifndef __HOURGLASS_H__
#define __HOURGLASS_H__

#include <string>
#include "Macros.h"

typedef unsigned char		uchar;
typedef unsigned int		uint;

struct NULL_API Hourglass																	// This struct is called Hourglass instead of Clock due to Linker conflicts with MathGeoLib.
{
	Hourglass(uint hours = 0, uchar minutes = 0, float seconds = 0.0f);

	void Update		(uint ms);
	void ResetClock	();

	//std::string GetTimeAsString(); //cant't return a temporary string across dll boundaries !!!! (runetime library is mt!!!)

	uint	hours;
	uchar	minutes;
	float	seconds;

	uint	previousTicks;
};

#endif // !__HOURGLASS_H__