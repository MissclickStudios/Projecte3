#include "VariableTypedefs.h"

#include "C_Animation.h"

C_Animation::C_Animation(GameObject* owner) : Component(owner, COMPONENT_TYPE::ANIMATION)
{

}

C_Animation::~C_Animation()
{

}

bool C_Animation::Update()
{
	bool ret = true;

	for (uint i = 0; i < clips.size(); ++i)
	{
		// UPDATE CLIPS
	}

	return ret;
}

bool C_Animation::CleanUp()
{
	bool ret = true;



	return ret;
}

bool C_Animation::SaveState(ParsonNode& root) const
{
	bool ret = true;



	return ret;
}

bool C_Animation::LoadState(ParsonNode& root)
{
	bool ret = true;



	return ret;
}

// --- C_ANIMATION METHODS
void C_Animation::AddClip(const AnimationClip& animation_clip)
{
	clips.push_back(animation_clip);
}