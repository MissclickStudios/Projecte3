#include "GameObject.h"

#include "AnimatorClip.h"

#include "AnimatorTrack.h"

AnimatorTrack::AnimatorTrack() :
name			("[NONE]"),
rootBone		(nullptr),
currentClip		(nullptr),
blendingClip	(nullptr)
{

}

AnimatorTrack::AnimatorTrack(const std::string& name, GameObject* rootBone, AnimatorClip* currentClip, AnimatorClip* blendingClip) :
name			(name),
rootBone		(rootBone),
currentClip		(currentClip),
blendingClip	(blendingClip)
{

}

AnimatorTrack::~AnimatorTrack()
{

}

bool AnimatorTrack::StepTrack()
{
	
	
	return true;
}

bool AnimatorTrack::CleanUp()
{
	return true;
}

// --- ANIMATOR TRACK METHODS
bool AnimatorTrack::IsActive() const
{
	return isActive;
}

void AnimatorTrack::Enable()
{
	isActive = true;
}

void AnimatorTrack::Disable()
{
	isActive = false;
}

const char* AnimatorTrack::GetName() const
{
	return name.c_str();
}

const GameObject* AnimatorTrack::GetRootBone() const
{
	return rootBone;
}

AnimatorClip* AnimatorTrack::GetCurrentClip() const
{
	return currentClip;
}

AnimatorClip* AnimatorTrack::GetBlendingClip() const
{
	return blendingClip;
}

void AnimatorTrack::SetName(const char* newName)
{
	name = newName;
}

void AnimatorTrack::SetRootBone(const GameObject* newRootBone)
{
	rootBone = newRootBone;
}

void AnimatorTrack::SetCurrentClip(AnimatorClip* newCurrentClip)
{
	currentClip = newCurrentClip;
}

void AnimatorTrack::SetBlendingClip(AnimatorClip* newBlendingClip)
{
	blendingClip = newBlendingClip;
}