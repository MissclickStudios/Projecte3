#include "Application.h"
#include "M_Camera3D.h"

#include "C_AudioListener.h"
#include "Component.h"
#include "C_Transform.h"
#include "C_Camera.h"
#include "GameObject.h"
#include "M_Audio.h"
#include "MathGeoLib/include/Math/float3.h"

C_AudioListener::C_AudioListener(GameObject* owner) : Component(owner, ComponentType::AUDIOLISTENER)
{
    float3 pos = owner->GetComponent<C_Transform>()->GetWorldPosition();

    wwiseObject = wwiseObject->CreateAudioListener(owner->GetUID(), owner->GetName(), pos);
}


C_AudioListener::~C_AudioListener()
{
}

bool C_AudioListener::Update()
{
    C_Transform* transform = this->GetOwner()->GetComponent<C_Transform>();
    float3 pos = transform->GetWorldPosition();
    float3 front = transform->GetWorldTransform().WorldZ();
    float3 up = transform->GetWorldTransform().WorldY();

    wwiseObject->SetPos(pos, front, up);

    return true;
}

bool C_AudioListener::CleanUp()
{
    return true;
}

bool C_AudioListener::SaveState(ParsonNode& root) const
{
    return true;
}

bool C_AudioListener::LoadState(ParsonNode& root)
{
    return true;
}