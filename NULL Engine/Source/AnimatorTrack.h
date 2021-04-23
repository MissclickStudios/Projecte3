#ifndef __ANIMATOR_TRACK_H__
#define __ANIMATOR_TRACK_H__

#include <string>
#include "Macros.h"

#include "BoneLink.h"

class AnimatorClip;

class GameObject;
class C_Transform;

struct Channel;

typedef unsigned int uint;

enum class TrackState
{
	PLAY,
	PAUSE,
	STEP,
	STOP
};

class NULL_API AnimatorTrack
{
public:
	AnimatorTrack();
	AnimatorTrack(const std::string& name, const GameObject* rootBone);
	~AnimatorTrack();

	bool StepTrack(float dt);
	bool CleanUp();

public:																																				// --- TRACK MANAGEMENT METHODS
	bool				Play						();
	bool				Pause						();
	bool				Step						();
	bool				Stop						();
	
	bool				PlayClip					(AnimatorClip* clip, std::vector<BoneLink>* clipBones, uint blendFrames = 0);

public:																																				// --- GET/SET METHODS
	TrackState			GetTrackState				();
	const char*			GetTrackStateAsString		();
	
	float				GetTrackSpeed				() const;
	void				SetTrackSpeed				(float newTrackSpeed);

	const char*			GetName						() const;
	const GameObject*	GetRootBone					() const;
	AnimatorClip*		GetCurrentClip				() const;
	AnimatorClip*		GetBlendingClip				() const;

	void				SetName						(const char* newName);
	void				SetRootBone					(const GameObject* newRootBone);
	bool				SetCurrentClip				(AnimatorClip* newClip, std::vector<BoneLink>* newBones);
	bool				SetBlendingClip				(AnimatorClip* newClip, std::vector<BoneLink>* newBones, uint newBlendFrames);

	bool				CurrentClipExists			() const;
	bool				BlendingClipExists			() const;

private:																																			// --- CLIP MANAGEMENT METHODS
	bool				StepClips					(float dt);

	void				SwitchBlendingToCurrent		();
	void				ResetCurrentBones			();

	bool				ValidateCurrentClip			();

private:																																			// --- BONE/CHANNEL UPDATE METHODS
	void				UpdateChannelTransforms		();

	Transform			GetPoseToPoseTransform		(uint tick, const Channel& channel, const C_Transform* originalTransform) const;						
	
	Transform			GetInterpolatedTransform	(double keyframe, const Channel& channel, const C_Transform* originalTransform) const;				
	const float3		GetInterpolatedPosition		(double keyframe, const Channel& channel) const;												
	const Quat			GetInterpolatedRotation		(double keyframe, const Channel& channel) const;												
	const float3		GetInterpolatedScale		(double keyframe, const Channel& channel) const;												
	
	Transform			GetBlendedTransform			(double bKeyframe, const Channel& bChannel, const Transform& originalTransform) const;			
	const float3		GetBlendedPosition			(double bKeyframe, const Channel& bChannel, float bRate, const float3& originalPosition) const;	
	const Quat			GetBlendedRotation			(double bKeyframe, const Channel& bChannel, float bRate, const Quat& originalRotation) const;	
	const float3		GetBlendedScale				(double bKeyframe, const Channel& bChannel, float bRate, const float3& originalScale) const;	

private:
	TrackState				trackState;
	float					trackSpeed;

	bool					interpolate;

	std::string				name;
	
	const GameObject*		rootBone;
	std::vector<BoneLink>	bones;
	
	AnimatorClip*			currentClip;
	AnimatorClip*			blendingClip;

	std::vector<BoneLink>*	currentBones;
	std::vector<BoneLink>*	blendingBones;

	uint					blendFrames;
};

#endif // !__ANIMATOR_TRACK_H__