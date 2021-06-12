#include "MathGeoTransform.h"
#include "JSONParser.h"
#include "Profiler.h"

#include "VariableTypedefs.h"
#include "Log.h"

#include "AnimatorClip.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "AnimatorTrack.h"

AnimatorTrack::AnimatorTrack() :
trackState		(TrackState::STOP),
trackSpeed		(1.0f),
interpolate		(true),
name			("[NONE]"),
rootBone		(nullptr),
rootBoneUID		(0),
currentClip		(nullptr),
blendingClip	(nullptr),
currentBones	(nullptr),
blendingBones	(nullptr),
blendFrames		(0)
{

}

AnimatorTrack::AnimatorTrack(const char* name, GameObject* rootBone, float trackSpeed, bool interpolate) :
trackState		(TrackState::STOP),
trackSpeed		(trackSpeed),
interpolate		(interpolate),
name			(name),
rootBone		(rootBone),
rootBoneUID		(0),
currentClip		(nullptr),
blendingClip	(nullptr),
currentBones	(nullptr),
blendingBones	(nullptr),
blendFrames		(0)
{
	if (rootBone != nullptr)
	{
		bones.emplace(rootBone->GetUID(), rootBone);
		
		std::vector<GameObject*> tmp;
		rootBone->GetAllChilds(tmp);
		for (auto object = tmp.cbegin(); object != tmp.cend(); ++object)
		{
			bones.emplace((*object)->GetUID(), (*object));
		}

		tmp.clear();
	}
}

AnimatorTrack::~AnimatorTrack()
{

}

bool AnimatorTrack::StepTrack(float dt)
{
	if (trackState == TrackState::PLAY || trackState == TrackState::STEP)
	{
		if (trackState == TrackState::STEP)
		{
			Pause();
		}
		
		bool success = StepClips(dt);
		if (success)
		{
			UpdateChannelTransforms();
		}
	}

	return true;
}

bool AnimatorTrack::CleanUp()
{
	rootBone = nullptr;
	bones.clear();
	
	currentClip		= nullptr;
	blendingClip	= nullptr;
	currentBones	= nullptr;
	blendingBones	= nullptr;

	return true;
}

bool AnimatorTrack::SaveState(ParsonNode& root) const
{
	root.SetString("Name", name.c_str());
	root.SetNumber("RootBoneUID", (double)((rootBone != nullptr) ? rootBone->GetUID() : 0));
	root.SetString("RootBoneName", (rootBone != nullptr) ? rootBone->GetName() : "[NONE]");

	root.SetNumber("TrackSpeed", (double)trackSpeed);
	root.SetBool("Interpolate", interpolate);
	
	return true;
}

bool AnimatorTrack::LoadState(const ParsonNode& root)
{
	name			= root.GetString("Name");
	rootBoneUID		= root.GetNumber("RootBoneUID");
	rootBoneName	= root.GetString("RootBoneName");

	trackSpeed		= (float)root.GetNumber("TrackSpeed");
	interpolate		= root.GetBool("Interpolate");
	
	return true;
}

// --- ANIMATOR TRACK METHODS
bool AnimatorTrack::Play()
{
	if (currentClip == nullptr && blendingClip == nullptr)
	{
		//LOG("[ERROR] Animator Track: Could not set Track State to { PLAY }! Error: Both currentClip and blendingClip were nullptr.");
		return false;
	}

	bool success = ValidateCurrentClip();
	if (!success)
	{
		LOG("[ERROR] Animator Track: Could not set Track State to { PLAY }! Error: currentClip could not be validated (currentClip != nullptr).");
		return false;
	}

	trackState = TrackState::PLAY;

	if (currentClip != nullptr)
		currentClip->playing = true;

	if (blendingClip != nullptr)
		blendingClip->playing = true;
	
	return (trackState == TrackState::PLAY);
}

bool AnimatorTrack::Pause()
{
	if (trackState != TrackState::PAUSE)
	{
		(trackState != TrackState::STOP) ? trackState = TrackState::PAUSE : LOG("[WARNING] Animator Track: Cannot Pause a Stopped Track!");
	}
	
	return (trackState == TrackState::PAUSE);
}

bool AnimatorTrack::Step()
{
	if (trackState != TrackState::STEP)
	{
		(trackState == TrackState::PAUSE) ? trackState = TrackState::STEP : LOG("[WARNING] Animator Track: Only Paused Tracks can be Stepped!");
	}
	
	return (trackState == TrackState::STEP);
}

bool AnimatorTrack::Stop()
{
	trackState = TrackState::STOP;

	if (currentClip != nullptr)
	{
		currentClip->playing = false;
		currentClip->ClearClip();
	}
	if (blendingClip != nullptr)
	{
		blendingClip->playing = false;
		blendingClip->ClearClip();
	}

	return (trackState == TrackState::STOP);
}

// --- CLIP MANAGEMENT METHODS (PUBLIC)
bool AnimatorTrack::PlayClip(AnimatorClip* clip, std::vector<BoneLink>* clipBones, uint blendFrames)
{
	if (clip == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Play Clip! Error: Given AnimatorClip* was nullptr.");
		return false;
	}
	if (clipBones == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Play Clip! Error: Given std::vector<BoneLink>* was nullptr");
		return false;
	}
	//if (currentClip != nullptr && currentClip->GetName() == clip->GetName())															// Cannot play the same clip twice.
	//{
	//	LOG("[WARNING] Animator Track: Cannot Play the same Clip twice!");
	//	return false;
	//}

	bool success = false;
	if (currentClip == nullptr || blendFrames == 0 || blendFrames > clip->GetDuration())
	{
		Stop();
		success = SetCurrentClip(clip, clipBones);
	}
	else
	{
		success = SetBlendingClip(clip, clipBones, blendFrames);
	}
	
	Play();

	return success;
}

bool AnimatorTrack::StepToPrevKeyframe()
{
	if (trackState == TrackState::PLAY)
	{
		LOG("[ERROR] Animator Track: Could not Step Animation to Prev Keyframe in Track { %s }! Error: Cannot Step an unpaused animation.", name.c_str());
		return false;
	}
	if (currentClip == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Step Animation to Prev Keyframe in Track { %s }! Error: Current Clip (AnimatorClip*) was nullptr.", name.c_str());
		return false;
	}
	if (currentBones == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Step Animation to Prev Keyframe in Track { %s }! Error: Current Bones is nullptr.", name.c_str());
		return false;
	}

	currentClip->StepClipToPrevKeyframe();

	for (auto bone = currentBones->cbegin(); bone != currentBones->cend(); ++bone)
	{
		C_Transform* cTransform = (*bone).gameObject->GetComponent<C_Transform>();
		if (cTransform == nullptr)
		{
			continue;
		}

		cTransform->ImportTransform(GetInterpolatedTransform((double)(currentClip->GetClipTick() + currentClip->GetStart()), (*bone).channel, cTransform));
	}
}

bool AnimatorTrack::StepToNextKeyframe()
{
	if (trackState == TrackState::PLAY)
	{
		LOG("[ERROR] Animator Track: Could not Step Animation to Next Keyframe in Track { %s }! Error: Cannot Step an unpaused animation.", name.c_str());
		return false;
	}
	if (currentClip == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Step Animation to Next Keyframe in Track { %s }! Error: Current Clip (AnimatorClip*) was nullptr.", name.c_str());
		return false;
	}
	if (currentBones == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Step Animation to Next Keyframe in Track { %s }! Error: Current Bones is nullptr", name.c_str());
		return false;
	}
	
	currentClip->StepClipToNextKeyframe();

	for (auto bone = currentBones->cbegin(); bone != currentBones->cend(); ++bone)
	{
		C_Transform* cTransform = (*bone).gameObject->GetComponent<C_Transform>();
		if (cTransform == nullptr)
		{
			continue;
		}

		cTransform->ImportTransform(GetInterpolatedTransform((double)(currentClip->GetClipTick() + currentClip->GetStart()), (*bone).channel, cTransform));
	}
}

bool AnimatorTrack::CurrentClipExists() const
{
	return (currentClip != nullptr);
}

bool AnimatorTrack::BlendingClipExists() const
{
	return (blendingClip != nullptr);
}

void AnimatorTrack::FreeCurrentClip()
{
	currentClip		= nullptr;
	currentBones	= nullptr;
}

void AnimatorTrack::FreeBlendingClip()
{
	blendingClip	= nullptr;
	blendingBones	= nullptr;
	blendFrames		= 0;
}

// -- GET/SET METHODS
TrackState AnimatorTrack::GetTrackState()
{
	return trackState;
}

const char* AnimatorTrack::GetTrackStateAsString()
{
	switch (trackState)
	{
	case TrackState::PLAY:	{ return "PLAY"; }	break;
	case TrackState::PAUSE: { return "PAUSE"; } break;
	case TrackState::STEP:	{ return "STEP"; }	break;
	case TrackState::STOP:	{ return "STOP"; }	break;
	}

	return "[NONE]";
}

float AnimatorTrack::GetTrackSpeed() const
{
	return trackSpeed;
}

void AnimatorTrack::SetTrackSpeed(float newTrackSpeed)
{
	trackSpeed = (newTrackSpeed >= 0.0f) ? newTrackSpeed : 0.0f;
}

bool AnimatorTrack::GetInterpolate() const
{
	return interpolate;
}

void AnimatorTrack::SetInterpolate(bool setTo)
{
	interpolate = setTo;
}

const char* AnimatorTrack::GetName() const
{
	return name.c_str();
}

GameObject* AnimatorTrack::GetRootBone() const
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
	if (newName == nullptr)
		return;
	
	name = newName;
}

void AnimatorTrack::SetRootBone(GameObject* newRootBone)
{
	if (newRootBone == nullptr || rootBone == newRootBone)
		return;
	
	rootBone = newRootBone;

	bones.clear();

	bones.emplace(rootBone->GetUID(), rootBone);

	std::vector<GameObject*> tmp;
	rootBone->GetAllChilds(tmp);
	for (auto object = tmp.cbegin(); object != tmp.cend(); ++object)
	{
		bones.emplace((*object)->GetUID(), (*object));
	}

	tmp.clear();
}

bool AnimatorTrack::SetCurrentClip(AnimatorClip* newClip, std::vector<BoneLink>* newBones)
{
	if (newClip == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Set Current Clip! Error: Given AnimatorClip* was nullptr.");
		return false;
	}
	if (newClip->GetAnimation() == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Set Current Clip! Error: Given AnimatorClip* had no R_Animation* assigned to it.");
		return false;
	}
	if (newBones == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Set Current Clip! Error: Given std::vector<BoneLink>* was nullptr.");
		return false;
	}

	currentClip		= newClip;
	currentBones	= newBones;

	return true;
}

bool AnimatorTrack::SetBlendingClip(AnimatorClip* newClip, std::vector<BoneLink>* newBones, uint newBlendFrames)
{
	if (blendingClip == newClip)
	{
		return false;
	}
	
	if (newClip == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Set Blending Clip! Error: Given AnimatorClip* was nullptr.");
		return false;
	}
	if (newClip->GetAnimation() == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Set Blending Clip! Error: Given AnimatorClip* had no R_Animation* assigned to it.");
		return false;
	}
	if (newBones == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Set Blending Clip! Error: Given std::vector<BoneLink>* was nullptr.");
		return false;
	}
	
	blendingClip	= newClip;
	blendingBones	= newBones;
	blendFrames		= newBlendFrames;

	blendingClip->ClearClip();																											// Re-setting the clip just in case.

	return true;
}

// --- CLIP MANAGEMENT METHODS (PRIVATE)
bool AnimatorTrack::StepClips(float dt)
{
	bool currentExists	= (currentClip != nullptr);
	bool blendingExists = (blendingClip != nullptr);
	if (!currentExists && !blendingExists)
	{
		LOG("[ERROR] Animator Component: Could not Step Clips! Error: There were no Current or Blending Clips set.");
		return false;
	}
	if (!currentExists && blendingExists)
	{
		SwitchBlendingToCurrent();
	}

	float stepValue = dt * trackSpeed;
	if (stepValue == 0.0f)
	{
		LOG("[WARNING] Animator Track: Step Value was 0.0f! dt: { %.3f } trackSpeed: { %.3f }", dt, trackSpeed);
		return false;
	}
	
	if (blendingClip != nullptr)
	{
		blendingClip->StepClip(stepValue);
		
		if (blendingClip->GetAnimationFrame() > (float)(blendingClip->GetStart() + blendFrames))										// ATTENTION HERE.
		{
			SwitchBlendingToCurrent();
			return true;
		}
	}

	if (currentClip != nullptr)
	{
		bool success = currentClip->StepClip(stepValue);
		if (!success && !currentClip->IsLooped())
		{			
			if (blendingClip != nullptr)
			{
				SwitchBlendingToCurrent();
				return true;
			}
			else
			{
				Stop();
				ResetCurrentBones();
				return false;
			}
		}
	}

	return true;
}

void AnimatorTrack::SwitchBlendingToCurrent()
{
	if (blendingClip == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Switch Blending To Current! Error: There was no Blending Clip to switch in.");
		return;
	}
	
	if (currentClip != nullptr)
	{
		currentClip->playing = false;
		currentClip->ClearClip();

		FreeCurrentClip();
	}

	SetCurrentClip(blendingClip, blendingBones);

	FreeBlendingClip();
}

void AnimatorTrack::ResetCurrentBones()
{
	if (currentClip == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Reset Current Bones of Track { %s }! Error: Current Clip was nullptr", name.c_str());
		return;
	}
	if (currentBones == nullptr)
	{
		LOG("[ERROR] Animator Track: Could not Reset Current Bones of Track { %s }! Error: Current Bones was nullptr", name.c_str());
		
		currentClip = nullptr;
		
		return;
	}

	for (auto bone = currentBones->cbegin(); bone != currentBones->cend(); ++bone)
	{
		bone->gameObject->transform->ImportTransform(GetInterpolatedTransform((double)currentClip->GetStart(), bone->channel, bone->gameObject->transform));
	}
}

bool AnimatorTrack::ValidateCurrentClip()
{
	if (currentClip == nullptr && blendingClip != nullptr)
	{
		SwitchBlendingToCurrent();
	}

	return (currentClip != nullptr);
}

// --- BONE/CHANNEL UPDATE METHODS
void AnimatorTrack::UpdateChannelTransforms()
{
	OPTICK_CATEGORY("Update Channel Transforms", Optick::Category::Animation);
	
	if (currentBones == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Update Channel Transforms! Error: Current Bones was nullptr.");
		return;
	}

	for (uint i = 0; i < currentBones->size(); ++i)
	{	
		const BoneLink& bone = currentBones->at(i);																									// (*currentBones)[i] would also be valid.
		if (bones.find(bone.gameObject->GetUID()) == bones.end())
		{
			continue;
		}

		if (interpolate)
		{
			Transform& iTransform = GetInterpolatedTransform(currentClip->GetAnimationFrame(), bone.channel, bone.gameObject->transform);
			
			if (blendingClip != nullptr && blendingBones != nullptr)
			{
				iTransform = GetBlendedTransform(blendingClip->GetAnimationFrame(), blendingBones->at(i).channel, iTransform);
			}

			bone.gameObject->transform->ImportTransform(iTransform);
		}
		else
		{
			if (!currentClip->inNewTick)
				continue;

			Transform& pTransform = GetPoseToPoseTransform(currentClip->GetAnimationTick(), bone.channel, bone.gameObject->transform);

			if (blendingClip != nullptr && blendingBones != nullptr)
			{
				pTransform = GetBlendedTransform(blendingClip->GetAnimationTick(), blendingBones->at(i).channel, pTransform);
			}

			bone.gameObject->transform->ImportTransform(pTransform);
		}
	}
}

Transform AnimatorTrack::GetPoseToPoseTransform(uint tick, const Channel& channel, const C_Transform* originalTransform) const
{
	const float3&	position	= (channel.HasPositionKeyframes())	? channel.GetPositionKeyframe(tick)->second : originalTransform->GetLocalPosition();
	const Quat&		rotation	= (channel.HasRotationKeyframes())	? channel.GetRotationKeyframe(tick)->second : originalTransform->GetLocalRotation();
	const float3&	scale		= (channel.HasScaleKeyframes())		? channel.GetScaleKeyframe(tick)->second	: originalTransform->GetLocalScale();
	
	return Transform(position, rotation, scale);
}

Transform AnimatorTrack::GetInterpolatedTransform(double keyframe, const Channel& channel, const C_Transform* originalTransform) const
{
	OPTICK_CATEGORY("Get Interpolated Transform", Optick::Category::Animation);

	const float3&	newPosition	= (channel.HasPositionKeyframes())	? GetInterpolatedPosition(keyframe, channel)	: originalTransform->GetLocalPosition();
	const Quat&		newRotation	= (channel.HasRotationKeyframes())	? GetInterpolatedRotation(keyframe, channel)	: originalTransform->GetLocalRotation();
	const float3&	newScale	= (channel.HasScaleKeyframes())		? GetInterpolatedScale(keyframe, channel)		: originalTransform->GetLocalScale();

	return Transform(newPosition, newRotation, newScale);
}

const float3 AnimatorTrack::GetInterpolatedPosition(double keyframe, const Channel& channel) const
{
	const PositionKeyframe& prevKeyframe = channel.GetClosestPrevPositionKeyframe(keyframe);
	const PositionKeyframe& nextKeyframe = channel.GetClosestNextPositionKeyframe(keyframe);

	if (prevKeyframe == nextKeyframe)
	{
		return prevKeyframe->second;
	}

	float rate	= (float)((keyframe - prevKeyframe->first) / (nextKeyframe->first - prevKeyframe->first));
	rate		= (rate > 1.0f) ? 1.0f : rate;																		// Safety Measure just in case rate is higher than 1.0f (100%).

	return (prevKeyframe->second.Lerp(nextKeyframe->second, rate));
}

const Quat AnimatorTrack::GetInterpolatedRotation(double keyframe, const Channel& channel) const
{
	const RotationKeyframe& prevKeyframe = channel.GetClosestPrevRotationKeyframe(keyframe);
	const RotationKeyframe& nextKeyframe = channel.GetClosestNextRotationKeyframe(keyframe);

	if (prevKeyframe == nextKeyframe)
	{
		return prevKeyframe->second;
	}

	float rate	= (float)((keyframe - prevKeyframe->first) / (nextKeyframe->first - prevKeyframe->first));
	rate		= (rate > 1.0f) ? 1.0f : rate;

	return (prevKeyframe->second.Slerp(nextKeyframe->second, rate));
}

const float3 AnimatorTrack::GetInterpolatedScale(double keyframe, const Channel& channel) const
{
	const ScaleKeyframe& prevKeyframe = channel.GetClosestPrevScaleKeyframe(keyframe);
	const ScaleKeyframe& nextKeyframe = channel.GetClosestNextScaleKeyframe(keyframe);

	if (prevKeyframe == nextKeyframe)
	{
		return prevKeyframe->second;
	}

	float rate	= (float)((keyframe - prevKeyframe->first) / (nextKeyframe->first - prevKeyframe->first));
	rate		= (rate > 1.0f) ? 1.0f : rate;

	return (prevKeyframe->second.Lerp(nextKeyframe->second, rate));
}

Transform AnimatorTrack::GetBlendedTransform(double bKeyframe, const Channel& bChannel, const Transform& originalTransform) const
{
	OPTICK_CATEGORY("Get Blended Transform", Optick::Category::Animation);
	
	if (!bChannel.HasPositionKeyframes() && !bChannel.HasRotationKeyframes() && !bChannel.HasScaleKeyframes()) { return originalTransform; }

	float bRate = (float)((bKeyframe - blendingClip->GetStart()) / blendFrames);
	bRate		= (bRate > 1.0f) ? 1.0f : bRate;

	const float3&	position	= (bChannel.HasPositionKeyframes()) ? originalTransform.position.Lerp((GetInterpolatedPosition(bKeyframe, bChannel)), bRate)	: originalTransform.position;
	const Quat&		rotation	= (bChannel.HasRotationKeyframes()) ? originalTransform.rotation.Slerp((GetInterpolatedRotation(bKeyframe, bChannel)), bRate)	: originalTransform.rotation;
	const float3&	scale		= (bChannel.HasScaleKeyframes())	? originalTransform.scale.Lerp((GetInterpolatedScale(bKeyframe, bChannel)), bRate)			: originalTransform.scale;

	return Transform(position, rotation, scale);
}

const float3 AnimatorTrack::GetBlendedPosition(double bKeyframe, const Channel& bChannel, float bRate, const float3& originalPosition) const
{
	return (originalPosition.Lerp((GetInterpolatedPosition(bKeyframe, bChannel)), bRate));		// We get the interpolated value of the keyframe-channel and then lerp origin to value.
}

const Quat AnimatorTrack::GetBlendedRotation(double bKeyframe, const Channel& bChannel, float bRate, const Quat& originalRotation) const
{
	return (originalRotation.Slerp((GetInterpolatedRotation(bKeyframe, bChannel)), bRate));
}

const float3 AnimatorTrack::GetBlendedScale(double bKeyframe, const Channel& bChannel, float bRate, const float3& originalScale) const
{
	return (originalScale.Lerp((GetInterpolatedScale(bKeyframe, bChannel)), bRate));
}
