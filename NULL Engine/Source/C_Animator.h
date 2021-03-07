#ifndef __C_ANIMATOR_H__
#define __C_ANIMATOR_H__

#include <string>
#include <vector>
#include <unordered_map>

#include "MathGeoTransform.h"

#include "BoneLink.h"
#include "AnimatorClip.h"

#include "Component.h"

namespace math
{
	class LineSegment;
}

class ParsonNode;

class GameObject;
class R_Animation;

struct Channel;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

struct AnimatorBones
{
	std::string name;
	std::vector<BoneLink> bones;
};

enum class InterpolationType																								// WIP IDEA
{
	LINEAL,
	CUSTOM
};

enum class AnimatorState
{
	PLAY,
	PAUSE,
	STEP,
	STOP
};

class NULL_API C_Animator : public Component																				// In charge of managing Skeletal Animation
{
public:
	C_Animator(GameObject* owner);
	~C_Animator();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::ANIMATOR; }

public:
	void			AddAnimation					(R_Animation* rAnimation);
	bool			AddClip							(const AnimatorClip& clip);
	void			PlayClip						(const std::string& clipName, uint blendFrames);
	
	bool			Play							();
	bool			Pause							();
	bool			Step							();
	bool			Stop							();

public:																														// --- DEBUG METHODS
	bool			StepToPrevKeyframe				();
	bool			StepToNextKeyframe				();
	bool			RefreshBoneDisplay				();

public:																														// --- CURRENT/BLENDING ANIMATION METHODS
	AnimatorClip*	GetCurrentClip					() const;
	AnimatorClip*	GetBlendingClip					() const;

	void			SetCurrentClip					(AnimatorClip* clip);
	void			SetBlendingClip					(AnimatorClip* clip, uint blendFrames);

	void			SetCurrentClipByIndex			(uint index);
	void			SetBlendingClipByIndex			(uint index, uint blendFrames);

	bool			CurrentClipExists				() const;
	bool			BlendingClipExists				() const;

	void			ClearCurrentClip				();
	void			ClearBlendingClip				();

public:																														// --- GET/SET METHODS
	std::vector<LineSegment> GetDisplayBones		() const;
	std::vector<std::string> GetClipNamesAsVector	() const;

	std::string		GetAnimatorStateAsString		() const;

	std::string		GetClipNamesAsString			() const;
	std::string		GetAnimationNamesAsString		() const;
	R_Animation*	GetAnimationByIndex				(uint index) const;

	float			GetPlaybackSpeed				() const;
	bool			GetInterpolate					() const;
	bool			GetLoopAnimation				() const;
	bool			GetPlayOnStart					() const;
	bool			GetCameraCulling				() const;
	bool			GetShowBones					() const;

	void			SetPlaybackSpeed				(float playbackSpeed);
	void			SetInterpolate					(bool setTo);
	void			SetLoopAnimation				(bool setTo);
	void			SetPlayOnStart					(bool setTo);
	void			SetCameraCulling				(bool setTo);
	void			SetShowBones					(bool setTo);
		 
private: 																									// --- INTERNAL METHODS
	void			AddAnimationsToAdd				();
	void			CheckGameState					();
	
	bool			StepAnimation					();
	bool			StepClips						();
	bool			BlendAnimation					();
	bool			ValidateCurrentClip				();
					
	void			SwitchBlendingToCurrent			();
	void			ResetBones						();
					
	bool			GenerateDefaultClip				(const R_Animation* rAnimation, AnimatorClip& defaultClip);

private:
	void			UpdateChannelTransforms			();
	void			UpdateMeshSkinning				(const GameObject* gameObject);
	void			UpdateDisplayBones				();
	void			GenerateBoneSegments			(const GameObject* bone);
	
	Transform		GetInterpolatedTransform		(double keyframe, const Channel& channel, const Transform& originalTransform) const;
	const float3	GetInterpolatedPosition			(double keyframe, const Channel& channel, const float3& originalPosition) const;
	const Quat		GetInterpolatedRotation			(double keyframe, const Channel& channel, const Quat& originalRotation) const;
	const float3	GetInterpolatedScale			(double keyframe, const Channel& channel, const float3& originalScale) const;

	Transform		GetPoseToPoseTransform			(uint tick, const Channel& channel, const Transform& originalTransform) const;
	const float3	GetPoseToPosePosition			(uint tick, const Channel& channel, const float3& originalPosition) const;
	const Quat		GetPoseToPoseRotation			(uint tick, const Channel& channel, const Quat& originalRotation) const;
	const float3	GetPoseToPoseScale				(uint tick, const Channel& channel, const float3& originalScale) const;

	Transform		GetBlendedTransform				(double blendingKeyframe, const Channel& blendingChannel, const Transform& originalTransform) const;
	const float3	GetBlendedPosition				(double blendingKeyframe, const Channel& blendingChannel, const float3& originalPosition) const;
	const Quat		GetBlendedRotation				(double blendingKeyframe, const Channel& blendingChannel, const Quat& originalRotation) const;
	const float3	GetBlendedScale					(double blendingKeyframe, const Channel& blendingChannel, const float3& originalScale) const;

	void			FindAnimationBones				(const R_Animation* rAnimation);
	bool			FindBoneLinks					(const R_Animation* rAnimation, std::vector<BoneLink>& links);
	GameObject*		FindRootBone					(const std::vector<BoneLink>& links);
	void			SetRootBone						(GameObject* rootBone);
	GameObject*		GetRootBone						() const;
		 
	void			SortBoneLinksByHierarchy		(const std::vector<BoneLink>& boneLinks, const GameObject* rootBone, std::vector<BoneLink>& sorted);

	void			GetAnimatedMeshes				();

private:
	std::vector<R_Animation*>						animations;											// Animation Resources. Contain bone information (transforms...).
	std::map<uint32, std::vector<BoneLink>>			animationBones;
	std::unordered_map<std::string, AnimatorClip>	clips;												// Segments of animations. "Idle", "Walk", "Attack"...
	
	std::vector<BoneLink>		currentBones;															// Multiple animations will have the same bones.
	std::vector<BoneLink>		blendingBones;

	std::vector<LineSegment>	displayBones;															// Line Segments between GO bones. For debug purposes.

	std::vector<R_Animation*>	animationsToAdd;														// TODO: Change this to something cleaner later.

	AnimatorClip*				currentClip;
	AnimatorClip*				blendingClip;

	GameObject*					currentRootBone;

	std::vector<C_Mesh*>		animatedMeshes;															// TMP. Until a better implementation is found;
	bool						foundAnimMeshes;														// -------------------------------------------- 

private:																								// --- FUNCTIONALITY VARIABLES
	AnimatorState	animatorState;

	uint			blendFrames;

private:																								// --- GET/SET VARIABLES	
	float			playbackSpeed;
	bool			interpolate;
	bool			loopAnimation;
	bool			playOnStart;
	bool			cameraCulling;
	
	bool			showBones;
};

#endif // !__C_ANIMATOR_H__