#include "C_AudioSource.h"
#include "Component.h"
#include "C_Transform.h"
#include "GameObject.h"
#include "M_Audio.h"
#include "MathGeoLib/include/Math/float3.h"

C_AudioSource::C_AudioSource(GameObject* owner) : Component(owner, ComponentType::AUDIOSOURCE)
{
	float3 pos = owner->GetComponent<C_Transform>()->GetWorldPosition();

	wwiseObject = wwiseObject->CreateAudioSource(owner->GetUID(), owner->GetName(), pos);
}

C_AudioSource::~C_AudioSource()
{
}

bool C_AudioSource::Update()
{
	GameObject* owner = GetOwner();

	float3 pos = owner->GetComponent<C_Transform>()->GetWorldPosition();

	wwiseObject->SetPos(pos);

	return true;
}

bool C_AudioSource::CleanUp()
{
	return true;
}

bool C_AudioSource::SaveState(ParsonNode& root) const
{
	return true;
}

bool C_AudioSource::LoadState(ParsonNode& root)
{
	return true;
}

void C_AudioSource::SetId(unsigned int id)
{
	this->id = id;
}

const unsigned int C_AudioSource::GetId()
{
	return id;
}

void C_AudioSource::PlayFx(unsigned int eventId)
{
	wwiseObject->PlayEvent(eventId);
}

void C_AudioSource::PauseFx(unsigned int eventId)
{
	wwiseObject->PauseEvent(eventId);
}

void C_AudioSource::ResumeFx(unsigned int eventId)
{
	wwiseObject->ResumeEvent(eventId);
}

void C_AudioSource::StopFx(unsigned int eventId)
{
	wwiseObject->StopEvent(eventId);
}

void C_AudioSource::SetVolume(float volume)
{
	wwiseObject->SetVolume(volume);
}

