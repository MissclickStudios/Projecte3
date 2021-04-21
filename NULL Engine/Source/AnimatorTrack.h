#ifndef __ANIMATOR_TRACK_H__
#define __ANIMATOR_TRACK_H__

#include <string>
#include "Macros.h"

#include "BoneLink.h"

class GameObject;
class AnimatorClip;

class NULL_API AnimatorTrack
{
public:
	AnimatorTrack();
	AnimatorTrack(const std::string& name, GameObject* rootBone, AnimatorClip* currentClip = nullptr, AnimatorClip* blendingClip = nullptr);
	~AnimatorTrack();

	bool StepTrack();
	bool CleanUp();

public:

public:
	bool				IsActive			() const;
	void				Enable				();
	void				Disable				();
	
	const char*			GetName				() const;
	const GameObject*	GetRootBone			() const;
	AnimatorClip*		GetCurrentClip		() const;
	AnimatorClip*		GetBlendingClip		() const;

	void				SetName				(const char* newName);
	void				SetRootBone			(const GameObject* newRootBone);
	void				SetCurrentClip		(AnimatorClip* newCurrentClip);
	void				SetBlendingClip		(AnimatorClip* newBlendingClip);

private:
	bool				isActive;
	
	std::string			name;
	
	const GameObject*	rootBone;
	
	AnimatorClip*		currentClip;
	AnimatorClip*		blendingClip;

	std::vector<BoneLink>* currentBones;
	std::vector<BoneLink>* blendingBones;
};

#endif // !__ANIMATOR_TRACK_H__