#ifndef __RANDOM_H__
#define __RANDOM_H__

#include "MathGeoLib/include/Algorithm/Random/LCG.h"
#include "VariableTypedefs.h"
#include "Macros.h"

namespace Random
{
	namespace LCG																// RNG library included with MathGeoLib. Average Performance (Real random factor...)
	{	
		static math::LCG lcgRand;

		uint32	MISSCLICK_API GetRandomUint();												// 
		uint32	MISSCLICK_API GetBoundedRandomUint(uint32 min = 0, uint32 max = 4294967295);	// 
		float	MISSCLICK_API GetBoundedRandomFloat(float min, float max);					// 
	}

	namespace PCG																// RNG library. Great Performance (Real random factor...)
	{
		void	MISSCLICK_API InitPCG();
		
		uint32	MISSCLICK_API GetRandomUint();												// 
		uint32	MISSCLICK_API GetBoundedRandomUint(uint32 max = 4294967295);					// 
		float	MISSCLICK_API GetBoundedRandomFloat(float max);								// 
	}
}

#endif // !__RANDOM_H__