#include "Particle.h"
#include "Particle.h"

Particle::Particle()
{

}

Particle::Particle(const Particle& particle)
{

}

Particle::~Particle()
{

}

void Particle::Update(float dt)
{

}

void Particle::ApplyBillboarding(Billboarding type)
{
	switch (type) {
	case Billboarding::AXIS_ALIGNED:
	break;
	case Billboarding::SCREEN_ALIGNED:
	break;
	case Billboarding::WORLD_ALIGNED:
	break;
	case Billboarding::NONE:
	break;
	}
}
