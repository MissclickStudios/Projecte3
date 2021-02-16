#ifndef __RANDOM_H__
#define __RANDOM_H__

#include "MathGeoLib/include/Algorithm/Random/LCG.h"
#include "VariableTypedefs.h"

namespace Random
{
	namespace LCG																// RNG library included with MathGeoLib. Average Performance (Real random factor...)
	{	
		static math::LCG lcg_rand;

		uint32	GetRandomUint();												// 
		uint32	GetBoundedRandomUint(uint32 min = 0, uint32 max = 4294967295);	// 
		float	GetBoundedRandomFloat(float min, float max);					// 
	}

	namespace PCG																// RNG library. Great Performance (Real random factor...)
	{
		void	InitPCG();
		
		uint32	GetRandomUint();												// 
		uint32	GetBoundedRandomUint(uint32 max = 4294967295);					// 
		float	GetBoundedRandomFloat(float max);								// 
	}
}

#endif // !__RANDOM_H__