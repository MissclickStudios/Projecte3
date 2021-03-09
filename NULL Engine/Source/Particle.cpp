#include "Particle.h"
#include "Particle.h"
#include "Application.h"
#include "C_Camera.h"
#include "M_Camera3D.h"

#include "MemoryManager.h"

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

void Particle::ScreenAligned(C_Camera* currentCamera)
{
	//particleMesh->SetRotation(currentCamera->GetMatrixRotation());
	//currentCamera->GetOGLViewMatrix().RotatePart();
}

void Particle::WorldAligned(C_Camera* currentCamera)
{
}

void Particle::AxisAligned(C_Camera* currentCamera)
{
}

void Particle::ApplyBillboarding(Billboarding type)
{
	switch (type) {
	case Billboarding::AXIS_ALIGNED:
		AxisAligned(App->camera->GetCurrentCamera());
	break;
	case Billboarding::SCREEN_ALIGNED:
		ScreenAligned(App->camera->GetCurrentCamera());
	break;
	case Billboarding::WORLD_ALIGNED:
		WorldAligned(App->camera->GetCurrentCamera());
	break;
	case Billboarding::NONE:
	break;
	}
}
