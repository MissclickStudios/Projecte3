#ifndef __ANIMATOR_TRACK_H__
#define __ANIMATOR_TRACK_H__

#include <string>
#include "Macros.h"

#include "BoneLink.h"

class ParsonNode;

class AnimatorClip;

class GameObject;
class C_Transform;

struct Channel;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

enum class TrackState
{
	PLAY,
	PAUSE,
	STEP,
	STOP
};

class MISSCLICK_API AnimatorTrack
{
public:
	AnimatorTrack();
	AnimatorTrack(const char* name, GameObject* rootBone, float trackSpeed = 1.0f, bool interpolate = true);
	~AnimatorTrack();

	bool StepTrack	(float dt);
	bool CleanUp	();

	bool SaveState	(ParsonNode& root) const;
	bool LoadState	(const ParsonNode& root);

public:																																				// --- TRACK MANAGEMENT METHODS
	bool				Play						();
	bool				Pause						();
	bool				Step						();
	bool				Stop						();

public:																																				// --- CLIP MANAGEMENT METHODS (PUBLIC)
	bool				PlayClip					(AnimatorClip* clip, std::vector<BoneLink>* clipBones, uint blendFrames = 0);
	
	bool				StepToPrevKeyframe			();
	bool				StepToNextKeyframe			();

	bool				CurrentClipExists			() const;
	bool				BlendingClipExists			() const;
	
	void				FreeCurrentClip				();
	void				FreeBlendingClip			();

public:																																				// --- GET/SET METHODS
	TrackState			GetTrackState				();
	const char*			GetTrackStateAsString		();
	
	float				GetTrackSpeed				() const;
	void				SetTrackSpeed				(float newTrackSpeed);

	bool				GetInterpolate				() const;
	void				SetInterpolate				(bool setTo);

	const char*			GetName						() const;
	GameObject*			GetRootBone					() const;
	AnimatorClip*		GetCurrentClip				() const;
	AnimatorClip*		GetBlendingClip				() const;

	void				SetName						(const char* newName);
	void				SetRootBone					(GameObject* newRootBone);
	bool				SetCurrentClip				(AnimatorClip* newClip, std::vector<BoneLink>* newBones);
	bool				SetBlendingClip				(AnimatorClip* newClip, std::vector<BoneLink>* newBones, uint newBlendFrames);

private:																																			// --- CLIP MANAGEMENT METHODS (PRIVATE)
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

public:
	uint32					rootBoneUID;															// TMP until a better solution to Save & Load the root bone is found.

private:
	TrackState				trackState;
	float					trackSpeed;
	bool					interpolate;

	std::string				name;
	
	GameObject*						rootBone;
	std::map<uint32, GameObject*>	bones;

	AnimatorClip*			currentClip;
	AnimatorClip*			blendingClip;
	std::vector<BoneLink>*	currentBones;
	std::vector<BoneLink>*	blendingBones;

	uint					blendFrames;
};

#endif // !__ANIMATOR_TRACK_H__