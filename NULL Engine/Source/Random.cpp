#include "MathGeoLib/include/Algorithm/Random/LCG.h"
#include "PCG/include/pcg_basic.h"

#include "Random.h"

// --- LCG ---
uint32 Random::LCG::GetRandomUint()
{	
	return (uint32)lcgRand.Int();
}

uint32 Random::LCG::GetBoundedRandomUint(uint32 min, uint32 max)
{
	return (uint32)lcgRand.Int(min, max);
}

float Random::LCG::GetBoundedRandomFloat(float min, float max)
{	
	return lcgRand.Float(min, max);
}

// --- PCG ---
void Random::PCG::InitPCG()
{
	pcg32_srandom(42u, 54u);
}

uint32 Random::PCG::GetRandomUint()
{
	return pcg32_random();
}

uint32 Random::PCG::GetBoundedRandomUint(uint32 max)
{
	/*pcg32_random_t rng;
	return pcg32_boundedrand_r(&rng, max);*/
	
	return pcg32_boundedrand(max);
}

float Random::PCG::GetBoundedRandomFloat(float max)
{
	return pcg32_boundedrand(max);
}