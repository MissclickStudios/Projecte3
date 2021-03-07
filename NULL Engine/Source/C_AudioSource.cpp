#include "Application.h"

#include "C_AudioSource.h"
#include "Component.h"
#include "C_Transform.h"
#include "GameObject.h"
#include "M_Audio.h"

#include "JSONParser.h"
#include "MathGeoLib/include/Math/float3.h"

#include "MemoryManager.h"


C_AudioSource::C_AudioSource(GameObject* owner) : Component(owner, ComponentType::AUDIOSOURCE)
{
	float3 pos = owner->GetComponent<C_Transform>()->GetWorldPosition();

	wwiseObject = wwiseObject->CreateAudioSource(owner->GetUID(), owner->GetName(), pos);

	eventInfo = { "None", 0 };
}

C_AudioSource::~C_AudioSource()
{
	for (std::vector<WwiseObject*>::iterator it = App->audio->audioSourceList.begin(); it != App->audio->audioSourceList.end(); ++it)
	{
		if ((*it)->GetWwiseObjectId() == wwiseObject->GetWwiseObjectId())
		{
			delete (*it);
			(*it) = nullptr;
			App->audio->audioSourceList.erase((it));
			break;
		}
	}
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
	root.SetNumber("Type", (uint)GetType());

	ParsonArray eventInfoArray = root.SetArray("Playing Event");

	eventInfoArray.SetString(eventInfo.first.c_str());
	eventInfoArray.SetNumber(eventInfo.second);


	return true;
}

bool C_AudioSource::LoadState(ParsonNode& root)
{
	ParsonArray eventInfoArray = root.GetArray("Playing Event");

	eventInfo.first = (std::string)eventInfoArray.GetString(0);
	eventInfo.second = (unsigned int)eventInfoArray.GetNumber(1);

	return true;
}

void C_AudioSource::SetEvent(std::string name,  unsigned int id)
{
	eventInfo.first = name;
	eventInfo.second = id;
}

const std::pair<std::string, unsigned int> C_AudioSource::GetEvent()
{
	return eventInfo;
}

void C_AudioSource::PlayFx(unsigned int eventId)
{
	wwiseObject->PlayEvent(eventId);
}

void C_AudioSource::PlayFx(std::string name)
{
	std::map<std::string, unsigned int>::const_iterator it = App->audio->eventMap.find(name);

	if (it != App->audio->eventMap.end())
	{
		wwiseObject->PlayEvent(eventInfo.second);
	}
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

float C_AudioSource::GetVolume()
{
	return wwiseObject->GetVolume();
}

void C_AudioSource::SetVolume(float volume)
{
	wwiseObject->SetVolume(volume);
}

